/*  OpenMesh v0.1.3ps (pre-stable)
 *  --------------------------------
 *  Works on real hardware.
 *  Survives RF noise.
 *  Does NOT tolerate stupidity.
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

#define PKT_DATA 0x01
#define PKT_ACK  0x02

#define ACK_TIMEOUT 600
#define ACK_RETRY_MAX 3

enum Preset { LONG_SLOW, LONG_FAST, MED_FAST, SHORT_FAST };

// ================= STRUCTS =================
struct MeshMsg {
  char text[20];
  uint32_t timestamp;
  bool isTX;
  bool active = false;
};

struct __attribute__((packed)) OpenMeshHeader {
  uint8_t version;
  uint8_t type;
  uint8_t ttl;
  uint16_t src;
  uint16_t dest;
  uint16_t msg_id;
  uint16_t payload_len;
};

struct PendingTX {
  uint16_t msg_id;
  uint16_t dest;
  char payload[20];
  uint8_t retries;
  uint32_t lastSend;
  bool active;
};

struct Neighbor {
  uint16_t id;
  int rssi;
  uint32_t lastSeen;
};

// ================= AES KEY =================
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

Preset currentP = LONG_SLOW;
long freq = 433000000L;

int lastRSSI = -128;
float lastSNR = 0;

uint32_t txPkts = 0;
uint32_t rxPkts = 0;

MeshMsg terminal[MSG_COUNT];
PendingTX pending[4];
Neighbor neighbors[5];
int neighborCount = 0;

// ================= SEEN CACHE =================
uint32_t seenMsgs[16];
uint8_t seenIdx = 0;

bool seen_before(uint16_t src, uint16_t msg_id) {
  uint32_t key = ((uint32_t)src << 16) | msg_id;
  for (int i = 0; i < 16; i++) {
    if (seenMsgs[i] == key) return true;
  }
  seenMsgs[seenIdx++] = key;
  if (seenIdx >= 16) seenIdx = 0;
  return false;
}

// ================= UTILS =================
void addTerminal(const char* msg, bool tx) {
  for (int i = 0; i < MSG_COUNT - 1; i++) terminal[i] = terminal[i + 1];
  strncpy(terminal[MSG_COUNT - 1].text, msg, 19);
  terminal[MSG_COUNT - 1].timestamp = millis();
  terminal[MSG_COUNT - 1].isTX = tx;
  terminal[MSG_COUNT - 1].active = true;
}

void applyLoRa() {
  LoRa.end();
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(freq)) return;

  if (currentP == LONG_SLOW) {
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(62500);
  } else if (currentP == LONG_FAST) {
    LoRa.setSpreadingFactor(11);
    LoRa.setSignalBandwidth(250000);
  } else if (currentP == MED_FAST) {
    LoRa.setSpreadingFactor(9);
    LoRa.setSignalBandwidth(250000);
  } else {
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(250000);
  }

  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(LORA_SYNCWORD);
  LoRa.enableCrc();
  LoRa.receive();
}

// ================= RELAY =================
void relay_packet(OpenMeshHeader& h,
                  uint8_t* iv,
                  uint8_t* tag,
                  uint8_t* ciphertext,
                  uint16_t len) {

  if (h.ttl == 0) return;
  if (h.src == nodeID) return;
  if (seen_before(h.src, h.msg_id)) return;

  h.ttl--;

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&h, sizeof(h));
  LoRa.write(iv, IV_SIZE);
  LoRa.write(tag, TAG_SIZE);
  LoRa.write(ciphertext, len);
  LoRa.endPacket(true);
  LoRa.receive();
}

// ================= CRYPTO SEND =================
void send_packet(uint8_t type, const char* msg, uint16_t dest, uint16_t msg_id) {
  int len = strlen(msg);
  uint8_t iv[IV_SIZE], tag[TAG_SIZE], ciphertext[64];

  for (int i = 0; i < IV_SIZE; i++) iv[i] = esp_random();

  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);
  mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);
  mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
    len, iv, IV_SIZE,
    NULL, 0,
    (const uint8_t*)msg,
    ciphertext,
    TAG_SIZE, tag);
  mbedtls_gcm_free(&gcm);

  OpenMeshHeader h = {
    2,
    type,
    MAX_TTL,
    nodeID,
    dest,
    msg_id,
    (uint16_t)len
  };

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&h, sizeof(h));
  LoRa.write(iv, IV_SIZE);
  LoRa.write(tag, TAG_SIZE);
  LoRa.write(ciphertext, len);
  LoRa.endPacket(true);
  LoRa.receive();
}

// ================= SEND WITH ACK =================
void secure_send(const char* msg, uint16_t dest) {
  uint16_t mid = random(1, 65535);

  for (int i = 0; i < 4; i++) {
    if (!pending[i].active) {
      pending[i] = { mid, dest, "", 0, millis(), true };
      strncpy(pending[i].payload, msg, 19);
      break;
    }
  }

  send_packet(PKT_DATA, msg, dest, mid);
  txPkts++;
  addTerminal(msg, true);
}

// ================= RX =================
void handle_rx() {
  int sz = LoRa.parsePacket();
  if (sz < sizeof(OpenMeshHeader) + IV_SIZE + TAG_SIZE) return;

  OpenMeshHeader h;
  LoRa.readBytes((uint8_t*)&h, sizeof(h));

  uint8_t iv[IV_SIZE], tag[TAG_SIZE], ciphertext[64], plaintext[64];
  LoRa.readBytes(iv, IV_SIZE);
  LoRa.readBytes(tag, TAG_SIZE);
  LoRa.readBytes(ciphertext, h.payload_len);

  lastRSSI = LoRa.packetRssi();
  lastSNR  = LoRa.packetSnr();
  rxPkts++;

  if (seen_before(h.src, h.msg_id)) return;

  mbedtls_gcm_context gcm;
  mbedtls_gcm_init(&gcm);
  mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);

  if (mbedtls_gcm_auth_decrypt(&gcm, h.payload_len,
      iv, IV_SIZE, NULL, 0,
      tag, TAG_SIZE,
      ciphertext, plaintext) != 0) {
    mbedtls_gcm_free(&gcm);
    return;
  }
  mbedtls_gcm_free(&gcm);

  plaintext[h.payload_len] = 0;

  // ===== DATA =====
  if (h.type == PKT_DATA) {

    if (h.dest == nodeID || h.dest == BROADCAST_ID) {
      addTerminal((char*)plaintext, false);
      SerialBT.printf("IN [%04X]: %s\n", h.src, plaintext);

      if (h.dest == nodeID) {
        char ackbuf[2];
        memcpy(ackbuf, &h.msg_id, 2);
        send_packet(PKT_ACK, ackbuf, h.src, h.msg_id);
      }
    }

    if (h.dest != nodeID) {
      relay_packet(h, iv, tag, ciphertext, h.payload_len);
    }
  }

  // ===== ACK (NO RELAY) =====
  if (h.type == PKT_ACK && h.dest == nodeID) {
    uint16_t acked;
    memcpy(&acked, plaintext, 2);

    for (int i = 0; i < 4; i++) {
      if (pending[i].active && pending[i].msg_id == acked) {
        pending[i].active = false;

        bool found = false;
        for (int n = 0; n < neighborCount; n++) {
          if (neighbors[n].id == h.src) {
            neighbors[n].rssi = lastRSSI;
            neighbors[n].lastSeen = millis();
            found = true;
          }
        }
        if (!found && neighborCount < 5) {
          neighbors[neighborCount++] = { h.src, lastRSSI, millis() };
        }
      }
    }
  }
}

// ================= RETRY =================
void handle_retries() {
  for (int i = 0; i < 4; i++) {
    if (!pending[i].active) continue;
    if (millis() - pending[i].lastSend > ACK_TIMEOUT) {
      if (pending[i].retries >= ACK_RETRY_MAX) {
        pending[i].active = false;
        continue;
      }
      pending[i].retries++;
      pending[i].lastSend = millis();
      send_packet(PKT_DATA, pending[i].payload, pending[i].dest, pending[i].msg_id);
    }
  }
}

// ================= SETUP / LOOP =================
void setup() {
  Serial.begin(115200);
  u8g2.begin();

  nodeID = (uint16_t)ESP.getEfuseMac();
  nodeName = "CORE-" + String(nodeID, HEX);
  nodeName.toUpperCase();
  SerialBT.begin(nodeName);

  applyLoRa();
  addTerminal("MESH ONLINE", true);
}

void loop() {
  button.tick();
  handle_rx();
  handle_retries();
}