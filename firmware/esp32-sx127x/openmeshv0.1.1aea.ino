// OpenMesh firmware – experimental, DIY-first
// Feel free to modify and improve under the project license.
// If you make big changes, please document them clearly.
//dont mess shit up ok i aint fixing it up
// If it works: great.
// If it breaks: congrats, you found a bug.
// PRs welcome. Panic optional.

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/aes.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= PROTOCOL & CONFIG =================
#define OPENMESH_MAX_PAYLOAD 180
#define BROADCAST_ID 0xFFFF
#define MAX_TTL 8 // TTL prevents your packet from touring the entire city
// Anti-loop exists because radios are stupid but persistent.

struct __attribute__((packed)) OpenMeshHeader {
    uint8_t  version; uint8_t type; uint8_t ttl; uint8_t flags;
    uint16_t src; uint16_t dest; uint16_t msg_id; uint16_t payload_len;
};

// Default LoRa Settings
struct LoRaSettings {
    long freq = 433E6;
    int sf = 12;
    long bw = 62.5E3;
    int tx = 20;
} currentCfg;

// AES-256 — yes this is real crypto, no this is not XOR, stop asking(Keep this identical on all your nodes or else you're fucked. AINT MY FAULT OK)
// And also No, this is not military grade.
// Yes, it's good enough for random people yelling "WYA".

unsigned char aes_key[32] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
};

// ================= HARDWARE PINS =================
#define BUTTON_PIN 13
#define OLED_SDA 21  // IF BLACK SCREEN: Change to 4 for Hel- no
#define OLED_SCL 22  // IF BLACK SCREEN: Change to 15 for Hel- nuh uh fuck them bro why you add this
#define LORA_SCK  18 // WHY DONT YOU ADD MULTIPLE BOARD SUPPORT -R
#define LORA_MISO 19 // BECAUSE IM LAZY - K
#define LORA_MOSI 23
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 26

//if you all asking for more board support maybe later I'm too lazy lmaoo

// ================= OBJECTS & STATE =================
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
OneButton button(BUTTON_PIN, true);
BluetoothSerial SerialBT;
Preferences prefs;

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

//now you can edit this word list make sure it fits

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
    Serial.printf("LoRa: %ldHz SF%d BW%ld\n", currentCfg.freq, currentCfg.sf, currentCfg.bw);
}

void generateAutoName() {
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    const char* adj[] = {"Neo", "Cyber", "Dark", "Echo", "Alpha", "Red", "Blue"};
    const char* noun[] = {"Node", "Link", "Mesh", "Wave", "Unit", "Point", "Base"};
    nodeName = String(adj[chipId % 7]) + "-" + String(noun[(chipId >> 8) % 7]) + "-" + String(chipId & 0xFF, HEX);
    nodeName.toUpperCase();
}

void encrypt_and_send(const char* msg, uint16_t dest) {
    lastMsgID++;
    OpenMeshHeader header = {1, 0x01, MAX_TTL, 0x01, nodeID, dest, lastMsgID, 16};
    unsigned char iv[16];
    for(int i=0; i<16; i++) iv[i] = (uint8_t)random(0, 255);
    unsigned char input[16] = {0}, output[16] = {0};
    strncpy((char*)input, msg, 15);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, aes_key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
    mbedtls_aes_free(&aes);

    LoRa.beginPacket();
    LoRa.write((uint8_t*)&header, sizeof(header));
    LoRa.write(iv, 16);
    LoRa.write(output, 16);
    LoRa.endPacket();
    SerialBT.printf("TX -> %u: %s\n", dest, msg);
}

void handleBT() {
    if (!SerialBT.available()) return;
    String cmd = SerialBT.readStringUntil('\n'); cmd.trim();
    if (cmd.startsWith("/sf ")) { currentCfg.sf = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/tx ")) { currentCfg.tx = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd.startsWith("/bw ")) { currentCfg.bw = cmd.substring(4).toInt(); applyLoRa(); }
    else if (cmd == "/id") { SerialBT.printf("NAME: %s | ID: %u\n", nodeName.c_str(), nodeID); }
}

// ================= UI & BUTTONS =================

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
        u8g2.setCursor(0,10); u8g2.print("DESTINATION:");
        for(int i=0; i<knownCount; i++) {
            u8g2.setCursor(0,25+i*11);
            u8g2.print(destIndex==i?"> ":"  "); u8g2.printf("NODE %u", knownNodes[i]);
        }
        u8g2.setCursor(0,25+knownCount*11);
        u8g2.print(destIndex==knownCount?"> ":"  "); u8g2.print("ALL NODES");
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
    if (sz >= sizeof(OpenMeshHeader) + 32) {
        OpenMeshHeader h;
        LoRa.readBytes((uint8_t*)&h, sizeof(h));
        uint8_t iv[16], enc[16];
        LoRa.readBytes(iv, 16); LoRa.readBytes(enc, 16);

        // Anti-Loop & ID 0 Filter
        if(h.src == 0 || h.src == nodeID) return;
        bool seen = false;
        for(int i=0; i<10; i++) if(seenMsgIDs[i] == h.msg_id) seen = true;
        if(seen) return;
        seenMsgIDs[seenIdx] = h.msg_id; seenIdx = (seenIdx + 1) % 10;

        // Discover Node
        bool known = false;
        for(int k=0; k<knownCount; k++) if(knownNodes[k] == h.src) known = true;
        if(!known && knownCount < 5) knownNodes[knownCount++] = h.src;

        // Decrypt
        if(h.dest == nodeID || h.dest == BROADCAST_ID) {
            unsigned char dec[16] = {0};
            mbedtls_aes_context aes;
            mbedtls_aes_init(&aes);
            mbedtls_aes_setkey_dec(&aes, aes_key, 256);
            mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv, enc, dec);
            mbedtls_aes_free(&aes);
            SerialBT.printf("IN [%u]: %s\n", h.src, dec);
        }

        // Relay
        if(h.dest != nodeID && h.ttl > 1) {
            delay(random(100, 500));// collision avoidance aka "please don't scream at the air"

            h.ttl--;
            LoRa.beginPacket();
            LoRa.write((uint8_t*)&h, sizeof(h));
            LoRa.write(iv, 16); LoRa.write(enc, 16);
            LoRa.endPacket();
        }
    }
}

//to all users DONT CANGE THE CODE
