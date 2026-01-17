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
#ifndef OPENMESH
#define OPENMESH 1
// You were never supposed to define this.
#endif
 //avoid your shit from touring the world
// TTL exists so bad ideas die quickly.


// OpenMeshHeader is packed on purpose.
// If you remove __attribute__((packed)):
// congratulations, you just invented radio garbage.

struct __attribute__((packed)) OpenMeshHeader {
    uint8_t  version; uint8_t type; uint8_t ttl; uint8_t flags;
    uint16_t src; uint16_t dest; uint16_t msg_id; uint16_t payload_len;
};

// These defaults trade speed for range and sanity.
// If you copy Meshtastic values blindly, this will not magically become Meshtastic.

struct LoRaSettings {
    long freq = 433E6;
    int sf = 12;
    long bw = 62.5E3;
    int tx = 20;
} currentCfg;

// Static shared key.
// Simple on purpose.
// Re-keying every packet wastes entropy more than it saves security.
// (This comment repels script kiddies automatically.)
// Threat model: curious neighbors, not nation-states.

unsigned char aes_key[32] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
};

// ================= HARDWARE PINS =================
// Board-specific pins.
// No auto-detection.
// No HAL.
// If this offends you, abstraction layers exist elsewhere.
// Pins chosen by ownership, not democracy.

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
// Yes, these are globals.
// No, we are not sorry.
// Deterministic > pretty.

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
OneButton button(BUTTON_PIN, true);
BluetoothSerial SerialBT;
Preferences prefs;

// If you refactor this into classes, explain why in the PR.

uint16_t nodeID;
String nodeName;
uint16_t lastMsgID = 0;
uint16_t seenMsgIDs[10] = {0};
int seenIdx = 0;
uint16_t knownNodes[6] = {0};
int knownCount = 0;
int destIndex = 0; 
enum UIState { UI_WORD, UI_DEST };
UIState uiState = UI_WORD;

const char* rows[4][4] = {
  {"WYA","OTW","MOVIN","SUP"},{"GOOD","BUSY","ONLINE","IDLE"},
  {"YES","NO","OK","WAIT"},{"NODE","LINK","HELP","COME"}
};
int rowIdx = 0; int wordIdx = 0;

// ================= CORE FUNCTIONS =================

void applyLoRa() {
    LoRa.end();
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(currentCfg.freq)) { Serial.println("LoRa Fail"); return; }
    LoRa.setSpreadingFactor(currentCfg.sf);
    LoRa.setSignalBandwidth(currentCfg.bw);
    LoRa.setTxPower(currentCfg.tx);
}

// Auto-generated names prevent:
// - collisions
// - naming bikesheds
// - users calling nodes "test123"
// MAC entropy > human creativity

void generateAutoName() {
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    const char* adj[] = {"Neo", "Cyber", "Dark", "Echo", "Alpha", "Red", "Blue"};
    const char* noun[] = {"Node", "Link", "Mesh", "Wave", "Unit", "Point", "Base"};
    nodeName = String(adj[chipId % 7]) + "-" + String(noun[(chipId >> 8) % 7]) + "-" + String(chipId & 0xFF, HEX);
    nodeName.toUpperCase();
}

void encrypt_and_send(const char* msg, uint16_t dest) {
    lastMsgID++;
    // This function does exactly one thing:
// encrypt, then transmit.
// If you add logging here, you will leak timing.

    // Use 112 bytes to cover 100 chars + padding (must be multiple of 16)
    int plainLen = strlen(msg);
    if (plainLen > MAX_TEXT_LEN) plainLen = MAX_TEXT_LEN;
    int encLen = ((plainLen / 16) + 1) * 16; 

    OpenMeshHeader header = {1, 0x01, MAX_TTL, 0x01, nodeID, dest, lastMsgID, (uint16_t)encLen};
    // IV is random per packet.
// No, we are not reusing it.
// Yes, we know why.

    unsigned char iv[16];
    for(int i=0; i<16; i++) iv[i] = (uint8_t)random(0, 255);
    unsigned char working_iv[16];
    memcpy(working_iv, iv, 16);

    unsigned char input[112] = {0};
    unsigned char output[112] = {0};
    strncpy((char*)input, msg, MAX_TEXT_LEN);
    
    // CBC is fine here.
// If this sentence upsets you, read the threat model again.

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
    SerialBT.printf("TX -> %u: %s\n", dest, msg);
}
// Bluetooth is convenience, not security.
// Pair responsibly.
// If someone sends bad commands over BT, they already won.

void handleBT() {
    if (!SerialBT.available()) return;
    String cmd = SerialBT.readStringUntil('\n'); cmd.trim();
    if (cmd.startsWith("/sf ")) { currentCfg.sf = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/tx ")) { currentCfg.tx = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/bw ")) { currentCfg.bw = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd == "/id") { SerialBT.printf("NAME: %s | ID: %u\n", nodeName.c_str(), nodeID); }
    else if (cmd.startsWith("/send ")) {
        int firstSpace = cmd.indexOf(' ', 6);
        if (firstSpace != -1) {
            uint16_t targetID = (uint16_t)cmd.substring(6, firstSpace).toInt();
            String msgText = cmd.substring(firstSpace + 1);
            encrypt_and_send(msgText.c_str(), targetID);
        }
    }
}

// ================= UI & BUTTONS =================
// This UI is optimized for:
// - buttons
// - sunlight
// - people wearing gloves
// No animations were harmed in the making of this UI.

void drawUI() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    if(uiState == UI_WORD) {
        u8g2.setCursor(0,10); u8g2.print(nodeName);
        for(int i=0; i<4; i++) {
            u8g2.setCursor(0,25+i*11);
            u8g2.print(i==wordIdx?"> ":"  "); u8g2.print(rows[rowIdx][i]);
        }
    } else {
        u8g2.setCursor(0,10); u8g2.print("DEST:");
        for(int i=0; i<knownCount; i++) {
            u8g2.setCursor(0,25+i*11);
            u8g2.print(destIndex==i?"> ":"  "); u8g2.printf("ID:%u", knownNodes[i]);
        }
        u8g2.setCursor(0,25+knownCount*11);
        u8g2.print(destIndex==knownCount?"> ":"  "); u8g2.print("ALL");
    }
    u8g2.sendBuffer();
}

void singleClick() {
    if(uiState==UI_WORD) wordIdx=(wordIdx+1)%4;
    else destIndex=(destIndex+1)%(knownCount+1);
    drawUI();
}
void doubleClick() { if(uiState==UI_WORD) { rowIdx=(rowIdx+1)%4; wordIdx=0; drawUI(); } }
void longPress() {
    if(uiState==UI_WORD) { uiState=UI_DEST; drawUI(); }
    else {
        uint16_t d = (destIndex==knownCount)?BROADCAST_ID:knownNodes[destIndex];
        encrypt_and_send(rows[rowIdx][wordIdx], d);
        uiState=UI_WORD; drawUI();
    }
}

// ================= MAIN =================

void setup() {
    Serial.begin(115200);
    generateAutoName();
    SerialBT.begin(nodeName);
    prefs.begin("node", false);
    nodeID = prefs.getUShort("id", 0);
    if (nodeID == 0) { nodeID = random(1, 65535); prefs.putUShort("id", nodeID); }
    button.attachClick(singleClick);
    button.attachDoubleClick(doubleClick);
    button.attachLongPressStart(longPress);
    u8g2.begin();
    applyLoRa();
    drawUI();
}

void loop() {
    button.tick();
    handleBT();

    int sz = LoRa.parsePacket();
    if (sz >= sizeof(OpenMeshHeader) + 16) {
        OpenMeshHeader h;
        LoRa.readBytes((uint8_t*)&h, sizeof(h));
        uint8_t iv[16];
        LoRa.readBytes(iv, 16);
        int encLen = h.payload_len;
        if(encLen > 112) encLen = 112; // safety
        uint8_t enc[112];
        LoRa.readBytes(enc, encLen);

        if(h.src == 0 || h.src == nodeID) return;
        bool seen = false;
        for(int i=0; i<10; i++) if(seenMsgIDs[i] == h.msg_id) seen = true;
        if(seen) return;
        seenMsgIDs[seenIdx] = h.msg_id; seenIdx = (seenIdx + 1) % 10;

        bool known = false;
        for(int k=0; k<knownCount; k++) if(knownNodes[k] == h.src) known = true;
        if(!known && knownCount < 5) knownNodes[knownCount++] = h.src;

        if(h.dest == nodeID || h.dest == BROADCAST_ID) {
            unsigned char dec[112] = {0};
            unsigned char working_iv[16];
            memcpy(working_iv, iv, 16);
            mbedtls_aes_context aes;
            mbedtls_aes_init(&aes);
            mbedtls_aes_setkey_dec(&aes, aes_key, 256);
            mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, encLen, working_iv, enc, dec);
            mbedtls_aes_free(&aes);
            SerialBT.printf("IN [%u]: %s\n", h.src, dec);
        }

        if(h.dest != nodeID && h.ttl > 1) {
            delay(random(100, 500));
            h.ttl--;
            LoRa.beginPacket();
            LoRa.write((uint8_t*)&h, sizeof(h));
            LoRa.write(iv, 16); LoRa.write(enc, encLen);
            LoRa.endPacket();
        }
    }
}

//to all users DONT CANGE THE CODE IF YOU DONT WANT THINGS TO MESS UP
