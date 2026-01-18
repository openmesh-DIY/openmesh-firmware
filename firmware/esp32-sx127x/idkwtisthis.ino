/*
 * OpenMesh v0.1.3ps (pre-stable)
 * ACK + TRUE RSSI edition
 * --------------------------------
 * Minimal surgery. No rewrites.
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/gcm.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= FIXED CONFIG =================

#define BROADCAST_ID 0xFFFF
#define MAX_TTL 8
#define LORA_SYNCWORD 0x12
#define IV_SIZE 12
#define TAG_SIZE 16
#define MSG_COUNT 4

#define PKT_DATA 1
#define PKT_ACK  2

enum Preset { LONG_SLOW, LONG_FAST, MED_FAST, SHORT_FAST };

struct MeshMsg {
  char text[20];
  uint32_t timestamp;
  bool isTX;
  bool active = false;
};

struct __attribute__((packed)) OpenMeshHeader {
  uint8_t  version;
  uint8_t  ttl;
  uint16_t src;
  uint16_t dest;
  uint16_t msg_id;
  uint16_t payload_len;
  uint8_t  type;
};

// AES-256-GCM KEY
unsigned char mesh_key[] = {
  0x1A,0x2B,0x3C,0x4D,0x5E,0x6F,0x70,0x81,
  0x92,0xA3,0xB4,0xC5,0xD6,0xE7,0xF8,0x09,
  0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
  0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00
};

// ================= HARDWARE =================

#define BUTTON_PIN 13
#define OLED_SDA 21
#define OLED_SCL 22
#define LORA_SCK 18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SS 5
#define LORA_RST 14
#define LORA_DIO0 26

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
OneButton button(BUTTON_PIN, true);
BluetoothSerial SerialBT;

// ================= STATE =================

uint16_t nodeID;
String nodeName;

int menuIdx = 0;
Preset currentP = LONG_SLOW;
long freq = 433000000L;

int lastRSSI = -128;
float lastSNR = 0;

uint32_t txPkts = 0;
uint32_t rxPkts = 0;

MeshMsg terminal[MSG_COUNT];

uint16_t neighbors[5];
int neighborCount = 0;

// ACK tracking
uint16_t waitingAck = 0;
uint32_t ackTimer = 0;

// ================= TERMINAL =================

void addTerminal(const char* msg, bool tx) {
  for (int i = 0; i < MSG_COUNT - 1; i++)
    terminal[i] = terminal[i + 1];

  strncpy(terminal[MSG_COUNT - 1].text, msg, 19);
  terminal[MSG_COUNT - 1].timestamp = millis();
  terminal[MSG_COUNT - 1].isTX = tx;
  terminal[MSG_COUNT - 1].active = true;
}

// ================= LORA CONFIG =================

void applyLoRa() {
  LoRa.end();
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(freq)) return;

  if (currentP == LONG_SLOW)   { LoRa.setSpreadingFactor(12); LoRa.setSignalBandwidth(62500); }
  if (currentP == LONG_FAST)   { LoRa.setSpreadingFactor(11); LoRa.setSignalBandwidth(250000); }
  if (currentP == MED_FAST)    { LoRa.setSpreadingFactor(9);  LoRa.setSignalBandwidth(250000); }
  if (currentP == SHORT_FAST)  { LoRa.setSpreadingFactor(7);  LoRa.setSignalBandwidth(250000); }

  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(LORA_SYNCWORD);
  LoRa.enableCrc();
  LoRa.receive();
}

// ================= CRYPTO SEND =================

void secure_send(const uint8_t* data, int len, uint16_t dest, uint8_t type, uint16_t msg_id) {

  uint8_t iv[IV_SIZE];
  uint8_t tag[TAG_SIZE];
  uint8_t ciphertext[128];

  for (int i = 0; i < IV_SIZE; i++) iv[i] = esp_random();

  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);
  mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);
  mbedtls_gcm_crypt_and_tag(
    &gcm, MBEDTLS_GCM_ENCRYPT,
    len, iv, IV_SIZE,
    NULL, 0,
    data, ciphertext,
    TAG_SIZE, tag
  );
  mbedtls_gcm_free(&gcm);

  OpenMeshHeader h = {
    2, MAX_TTL, nodeID, dest,
    msg_id, (uint16_t)len, type
  };

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&h, sizeof(h));
  LoRa.write(iv, IV_SIZE);
  LoRa.write(tag, TAG_SIZE);
  LoRa.write(ciphertext, len);
  LoRa.endPacket(true);
  LoRa.receive();
}

// ================= UI =================

void drawUI() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.drawBox(0,0,128,12);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setCursor(2,9);
  u8g2.print(nodeName);
  u8g2.setCursor(95,9);
  u8g2.print(freq/1000000.0,1);
  u8g2.setDrawColor(1);

  for (int i = 0; i < MSG_COUNT; i++) {
    if (!terminal[i].active) continue;
    u8g2.setCursor(0, 25 + i * 10);
    u8g2.print(terminal[i].isTX ? "TX:" : "RX:");
    u8g2.setCursor(30, 25 + i * 10);
    u8g2.print(terminal[i].text);
  }

  u8g2.setCursor(0, 63);
  u8g2.printf("RSSI:%d SNR:%.1f", lastRSSI, lastSNR);
  u8g2.sendBuffer();
}

// ================= SETUP =================

void setup() {
  u8g2.begin();
  Serial.begin(115200);

  nodeID = (uint16_t)ESP.getEfuseMac();
  nodeName = "CORE-" + String(nodeID, HEX);
  nodeName.toUpperCase();

  SerialBT.begin(nodeName);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(freq)) while (1);

  applyLoRa();
  addTerminal("MESH ONLINE", true);
  drawUI();
}

// ================= LOOP =================

void loop() {

  int sz = LoRa.parsePacket();
  if (sz >= sizeof(OpenMeshHeader) + IV_SIZE + TAG_SIZE) {

    int rxRSSI = LoRa.packetRssi();
    float rxSNR = LoRa.packetSnr();

    OpenMeshHeader h;
    LoRa.readBytes((uint8_t*)&h, sizeof(h));

    uint8_t iv[IV_SIZE], tag[TAG_SIZE];
    uint8_t ciphertext[128], plaintext[128];

    LoRa.readBytes(iv, IV_SIZE);
    LoRa.readBytes(tag, TAG_SIZE);
    LoRa.readBytes(ciphertext, h.payload_len);

    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);

    if (mbedtls_gcm_auth_decrypt(
          &gcm, h.payload_len,
          iv, IV_SIZE,
          NULL, 0,
          tag, TAG_SIZE,
          ciphertext, plaintext) == 0) {

      if (h.type == PKT_ACK && h.msg_id == waitingAck) {
        lastRSSI = (int8_t)plaintext[0];
        lastSNR  = ((int8_t)plaintext[1]) / 2.0;
        waitingAck = 0;
      }

      if (h.type == PKT_DATA) {
        plaintext[h.payload_len] = 0;
        addTerminal((char*)plaintext, false);
        rxPkts++;

        // SEND ACK WITH REAL RSSI
        int8_t ackPayload[2] = {
          (int8_t)rxRSSI,
          (int8_t)(rxSNR * 2)
        };
        secure_send((uint8_t*)ackPayload, 2, h.src, PKT_ACK, h.msg_id);
      }
    }

    mbedtls_gcm_free(&gcm);
    drawUI();
  }
}