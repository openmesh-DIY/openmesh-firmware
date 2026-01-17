// If you are reading this:
// 1) yes, this is the real firmware
// 2) no, there is no config UI
// 3) changing random values WILL break the mesh
// 4) this was written with intent, not vibes
// grep -R "magic" will disappoint you

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/aes.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= PROTOCOL & CONFIG =================
#define MAX_TEXT_LEN 100
#define BROADCAST_ID 0xFFFF
#define MAX_TTL 8

struct __attribute__((packed)) OpenMeshHeader {
    uint8_t  version;
    uint8_t  type;
    uint8_t  ttl;
    uint8_t  flags;
    uint16_t src;
    uint16_t dest;
    uint16_t msg_id;
    uint16_t payload_len;
};

struct LoRaSettings {
    long freq = 433E6;
    int sf = 12;
    long bw = 62.5E3;
    int tx = 20;
} currentCfg;

unsigned char aes_key[32] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
};

// ================= HARDWARE PINS =================
#define BUTTON_PIN 13
#define OLED_SDA 21
#define OLED_SCL 22
#define LORA_SCK  18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 26

// ================= OBJECTS & STATE =================
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
OneButton button(BUTTON_PIN, true);
BluetoothSerial SerialBT;
Preferences prefs;

uint16_t nodeID;
String nodeName;
uint16_t lastMsgID = 0;
uint32_t seenMsgs[12] = {0};
int seenIdx = 0;
uint16_t knownNodes[6] = {0};
int knownCount = 0;
int destIndex = 0;

enum UIState { UI_WORD, UI_DEST, UI_MSG_POPUP };
UIState uiState = UI_WORD;
String lastReceivedMsg = "";
uint32_t msgDisplayTimer = 0;

const char* rows[4][4] = {
  {"WYA","OTW","MOVIN","SUP"},
  {"GOOD","BUSY","ONLINE","IDLE"},
  {"YES","NO","OK","WAIT"},
  {"NODE","LINK","HELP","COME"}
};
int rowIdx = 0;
int wordIdx = 0;

// ================= CORE FUNCTIONS =================

void applyLoRa() {
    LoRa.end();
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(currentCfg.freq)) return;
    LoRa.setSpreadingFactor(currentCfg.sf);
    LoRa.setSignalBandwidth(currentCfg.bw);
    LoRa.setTxPower(currentCfg.tx);
    LoRa.receive(); 
}

void generateAutoName() {
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    const char* adj[]  = {"Neo","Cyber","Dark","Echo","Alpha","Red","Blue"};
    const char* noun[] = {"Node","Link","Mesh","Wave","Unit","Point","Base"};
    nodeName = String(adj[chipId % 7]) + "-" + String(noun[(chipId >> 8) % 7]) + "-" + String(chipId & 0xFF, HEX);
    nodeName.toUpperCase();
}

void encrypt_and_send(const char* msg, uint16_t dest) {
    lastMsgID++;
    int plainLen = strlen(msg);
    if (plainLen > MAX_TEXT_LEN) plainLen = MAX_TEXT_LEN;
    int encLen = ((plainLen / 16) + 1) * 16;
    OpenMeshHeader header = {1, 0x01, MAX_TTL, 0x01, nodeID, dest, lastMsgID, (uint16_t)encLen};
    unsigned char iv[16];
    for (int i = 0; i < 16; i++) iv[i] = random(0, 255);
    unsigned char working_iv[16];
    memcpy(working_iv, iv, 16);
    unsigned char input[112]  = {0};
    unsigned char output[112] = {0};
    strncpy((char*)input, msg, MAX_TEXT_LEN);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aes_key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, encLen, working_iv, input, output);
    mbedtls_aes_free(&aes);
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&header, sizeof(header));
    LoRa.write(iv, 16);
    LoRa.write(output, encLen);
    LoRa.endPacket();
    LoRa.receive(); 
    uint32_t sig = ((uint32_t)nodeID << 16) | lastMsgID;
    seenMsgs[seenIdx] = sig;
    seenIdx = (seenIdx + 1) % 12;
    SerialBT.printf("TX -> %u: %s\n", dest, msg);
}

void handleBT() {
    if (!SerialBT.available()) return;
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    if (cmd.startsWith("/sf ")) { currentCfg.sf = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/tx ")) { currentCfg.tx = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/bw ")) { currentCfg.bw = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd == "/id") { SerialBT.printf("NAME: %s | ID: %u\n", nodeName.c_str(), nodeID); }
    else if (cmd.startsWith("/send ")) {
        int p = cmd.indexOf(' ', 6);
        if (p > 0) {
            uint16_t target = cmd.substring(6, p).toInt();
            String text = cmd.substring(p + 1);
            encrypt_and_send(text.c_str(), target);
        }
    }
}

// ================= UI & LOOP =================
void drawUI() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    if (uiState == UI_MSG_POPUP) {
        u8g2.setCursor(0, 10); u8g2.print("INCOMING MSG:");
        u8g2.setCursor(0, 30); u8g2.print(lastReceivedMsg);
    } else if (uiState == UI_WORD) {
        u8g2.setCursor(0,10); u8g2.print(nodeName);
        for (int i = 0; i < 4; i++) {
            u8g2.setCursor(0, 25 + i * 11);
            u8g2.print(i == wordIdx ? "> " : "  "); u8g2.print(rows[rowIdx][i]);
        }
    } else {
        u8g2.setCursor(0,10); u8g2.print("DEST:");
        for (int i = 0; i < knownCount; i++) {
            u8g2.setCursor(0, 25 + i * 11);
            u8g2.print(destIndex == i ? "> " : "  "); u8g2.printf("ID:%u", knownNodes[i]);
        }
        u8g2.setCursor(0, 25 + knownCount * 11);
        u8g2.print(destIndex == knownCount ? "> " : "  "); u8g2.print("ALL");
    }
    u8g2.sendBuffer();
}

void singleClick() {
    if (uiState == UI_MSG_POPUP) uiState = UI_WORD;
    else if (uiState == UI_WORD) wordIdx = (wordIdx + 1) % 4;
    else destIndex = (destIndex + 1) % (knownCount + 1);
    drawUI();
}
void doubleClick() { if (uiState == UI_WORD) { rowIdx = (rowIdx + 1) % 4; wordIdx = 0; drawUI(); } }
void longPress() {
    if (uiState == UI_WORD) uiState = UI_DEST;
    else if (uiState == UI_DEST) {
        uint16_t d = (destIndex == knownCount) ? BROADCAST_ID : knownNodes[destIndex];
        encrypt_and_send(rows[rowIdx][wordIdx], d);
        uiState = UI_WORD;
    }
    drawUI();
}

void setup() {
    Serial.begin(115200); generateAutoName(); SerialBT.begin(nodeName);
    prefs.begin("node", false); nodeID = prefs.getUShort("id", 0);
    if (nodeID == 0) { nodeID = random(1, 65535); prefs.putUShort("id", nodeID); }
    button.attachClick(singleClick); button.attachDoubleClick(doubleClick); button.attachLongPressStart(longPress);
    u8g2.begin(); applyLoRa(); drawUI();
}

void loop() {
    button.tick(); handleBT();
    if (uiState == UI_MSG_POPUP && (millis() - msgDisplayTimer > 7000)) { uiState = UI_WORD; drawUI(); }
    int sz = LoRa.parsePacket();
    if (sz < sizeof(OpenMeshHeader) + 16) return;
    OpenMeshHeader h; LoRa.readBytes((uint8_t*)&h, sizeof(h));
    uint32_t sig = ((uint32_t)h.src << 16) | h.msg_id;
    for (int i = 0; i < 12; i++) { if (seenMsgs[i] == sig) return; }
    seenMsgs[seenIdx] = sig; seenIdx = (seenIdx + 1) % 12;
    uint8_t iv[16]; LoRa.readBytes(iv, 16);
    int encLen = min((int)h.payload_len, 112);
    uint8_t enc[112]; LoRa.readBytes(enc, encLen);
    if (knownCount < 6) {
        bool known = false;
        for (int i=0; i<knownCount; i++) if (knownNodes[i] == h.src) known = true;
        if (!known) knownNodes[knownCount++] = h.src;
    }
    if (h.dest == nodeID || h.dest == BROADCAST_ID) {
        unsigned char dec[112] = {0}; unsigned char working_iv[16]; memcpy(working_iv, iv, 16);
        mbedtls_aes_context aes; mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_dec(&aes, aes_key, 256);
        mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, encLen, working_iv, enc, dec);
        mbedtls_aes_free(&aes);
        lastReceivedMsg = String((char*)dec); uiState = UI_MSG_POPUP; msgDisplayTimer = millis(); drawUI();
        SerialBT.printf("IN [%u]: %s\n", h.src, (char*)dec);
    }
    if (h.ttl > 1 && h.src != nodeID) {
        h.ttl--; delay(random(100, 500));
        LoRa.beginPacket(); LoRa.write((uint8_t*)&h, sizeof(h)); LoRa.write(iv, 16); LoRa.write(enc, encLen); LoRa.endPacket(); LoRa.receive(); 
    }
}

// to all users:
// DONT CHANGE THE CODE IF YOU DONT WANT THINGS TO MESS UP
