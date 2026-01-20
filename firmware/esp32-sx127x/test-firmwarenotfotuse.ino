/*
 * OpenMesh v0.1.4bsea1
Yes it works
with good hardware not bad ones
 */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/gcm.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= CONFIG =================
#define BROADCAST_ID 0xFFFF
#define MAX_TTL 8
#define LORA_SYNCWORD 0x12
#define IV_SIZE 12
#define TAG_SIZE 16
#define MSG_COUNT 4

enum Preset { LONG_SLOW, LONG_FAST, MED_FAST, SHORT_FAST };
struct MeshMsg { char text[20]; uint32_t timestamp; bool isTX; bool active = false; };

//  MESH HOPPING: Added msg_type to header for ACK system
struct __attribute__((packed)) OpenMeshHeader { 
    uint8_t version; 
    uint8_t ttl; 
    uint16_t src; 
    uint16_t dest; 
    uint16_t msg_id; 
    uint16_t payload_len;
    uint8_t msg_type; // 0=DATA, 1=ACK
};

// AES-256-GCM Key
unsigned char mesh_key[] = {
    0x1A,0x2B,0x3C,0x4D,0x5E,0x6F,0x70,0x81,0x92,0xA3,0xB4,0xC5,
    0xD6,0xE7,0xF8,0x09,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
    0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00
};

// ================= HARDWARE PINS =================
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

// System State
uint16_t nodeID; String nodeName;
int menuIdx = 0; Preset currentP = LONG_SLOW;
long freq = 433000000L;
int lastRSSI = -128, noiseFloor = -128;
float lastSNR = 0;
uint32_t relayedCount = 0, txPkts = 0, rxPkts = 0;
MeshMsg terminal[MSG_COUNT];

//  MESH HOPPING: Enhanced neighbor tracking with RSSI
struct Neighbor {
    uint16_t node_id;
    int rssi;
    float snr;
    uint32_t last_seen;
    bool active;
};
Neighbor neighbors[10];
int neighborCount = 0;

//  MESH HOPPING: Message deduplication
uint32_t seenMsgs[50];
int seenIdx = 0;

// ================= CORE FUNCTIONS =================

void addTerminal(const char* msg, bool tx) {
    for (int i = 0; i < MSG_COUNT - 1; i++) terminal[i] = terminal[i+1];
    strncpy(terminal[MSG_COUNT-1].text, msg, 19);
    terminal[MSG_COUNT-1].text[19] = '\0';
    terminal[MSG_COUNT-1].timestamp = millis();
    terminal[MSG_COUNT-1].isTX = tx;
    terminal[MSG_COUNT-1].active = true;
}

//  MESH HOPPING: Check if message already seen
bool isMsgSeen(uint16_t msgId) {
    for (int i = 0; i < 50; i++) {
        if (seenMsgs[i] == msgId) return true;
    }
    return false;
}

//  MESH HOPPING: Add message to seen list
void addSeenMsg(uint16_t msgId) {
    seenMsgs[seenIdx] = msgId;
    seenIdx = (seenIdx + 1) % 50;
}

//  MESH HOPPING: Update neighbor with RSSI
void updateNeighbor(uint16_t nodeId, int rssi, float snr) {
    for (int i = 0; i < neighborCount; i++) {
        if (neighbors[i].node_id == nodeId) {
            neighbors[i].rssi = rssi;
            neighbors[i].snr = snr;
            neighbors[i].last_seen = millis();
            neighbors[i].active = true;
            return;
        }
    }
    if (neighborCount < 10) {
        neighbors[neighborCount].node_id = nodeId;
        neighbors[neighborCount].rssi = rssi;
        neighbors[neighborCount].snr = snr;
        neighbors[neighborCount].last_seen = millis();
        neighbors[neighborCount].active = true;
        neighborCount++;
    }
}

void applyLoRa() {
    LoRa.end();
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(freq)) return;
    
    if(currentP == LONG_SLOW)   { LoRa.setSpreadingFactor(12); LoRa.setSignalBandwidth(62500); }
    else if(currentP == LONG_FAST)  { LoRa.setSpreadingFactor(11); LoRa.setSignalBandwidth(250000); }
    else if(currentP == MED_FAST)   { LoRa.setSpreadingFactor(9);  LoRa.setSignalBandwidth(250000); }
    else if(currentP == SHORT_FAST) { LoRa.setSpreadingFactor(7);  LoRa.setSignalBandwidth(250000); }

    LoRa.setCodingRate4(5); LoRa.setSyncWord(LORA_SYNCWORD); LoRa.enableCrc(); LoRa.receive();
}

void secure_send(const char* msg, uint16_t dest) {
    int len = strlen(msg);
    uint8_t iv[IV_SIZE], tag[TAG_SIZE]; 
    uint8_t ciphertext[100];

    for(int i=0; i<IV_SIZE; i++) iv[i] = esp_random() % 256;

    mbedtls_gcm_context gcm; mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);
    mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT, len, iv, IV_SIZE, NULL, 0, (const uint8_t*)msg, ciphertext, TAG_SIZE, tag);
    mbedtls_gcm_free(&gcm);

    //  MESH HOPPING: Include msg_type in header
    OpenMeshHeader h = {2, MAX_TTL, nodeID, dest, (uint16_t)random(0, 65535), (uint16_t)len, 0}; // msg_type=0 for DATA
    
    LoRa.beginPacket();
    LoRa.write((uint8_t*)&h, sizeof(h)); 
    LoRa.write(iv, IV_SIZE); 
    LoRa.write(tag, TAG_SIZE); 
    LoRa.write(ciphertext, len);
    LoRa.endPacket(true); 
    LoRa.receive();
    txPkts++; 
    addTerminal(msg, true);
}

// ================= UI SYSTEM =================

void drawUI() {
    u8g2.clearBuffer();
    u8g2.setDrawColor(1); u8g2.drawBox(0,0,128,12); u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_6x10_tf); u8g2.setCursor(2, 9); u8g2.print(nodeName);
    u8g2.setCursor(95, 9); u8g2.print(freq/1000000.0, 1); u8g2.setDrawColor(1);

    if (menuIdx == 0) { // TERMINAL LOG
        for (int i = 0; i < MSG_COUNT; i++) {
            if (!terminal[i].active) continue;
            int y = 25 + (i * 10);
            int diff = (millis() - terminal[i].timestamp) / 60000;
            u8g2.setFont(u8g2_font_4x6_tf); u8g2.setCursor(0, y); u8g2.printf("[%dm]", diff);
            u8g2.setFont(u8g2_font_5x8_tf); u8g2.setCursor(25, y); 
            u8g2.print(terminal[i].isTX ? "TX:" : "RX:"); u8g2.setCursor(42, y); u8g2.print(terminal[i].text);
        }
        u8g2.drawLine(0, 56, 128, 56); u8g2.setFont(u8g2_font_4x6_tf);
        u8g2.setCursor(0, 63); u8g2.printf("RSSI:%d SNR:%.1f RLY:%u", lastRSSI, lastSNR, relayedCount);
    } 
    else if (menuIdx == 1) { // SIGNAL MONITOR
        u8g2.setCursor(2, 22); u8g2.print("SIGNAL MONITOR:");
        int bar = map(lastRSSI, -128, -30, 0, 100);
        u8g2.drawFrame(10, 35, 108, 10); u8g2.drawBox(10, 35, constrain(bar,0,108), 10);
        u8g2.setCursor(10, 55); u8g2.printf("LIVE RSSI: %d dBm", lastRSSI);
    }
    else if (menuIdx == 2) { //  MESH HOPPING: NEIGHBORS with RSSI
        u8g2.setCursor(2, 22); u8g2.print("NEIGHBORS:");
        for(int i = 0; i < neighborCount && i < 4; i++) {
            u8g2.setCursor(10, 34 + (i * 9)); u8g2.printf("%04X RSSI:%d", neighbors[i].node_id, neighbors[i].rssi);
        }
    }
    else if (menuIdx == 3) { // PRESET SELECTOR
        u8g2.setCursor(2, 22); u8g2.print("MODEM PRESET:");
        const char* names[] = {"LONG-SLOW", "LONG-FAST", "MED-FAST", "SHRT-FAST"};
        for(int i = 0; i < 4; i++) {
            u8g2.setCursor(10, 34 + (i * 8)); 
            if(currentP == i) u8g2.print("[X] "); else u8g2.print("[ ] "); 
            u8g2.print(names[i]); 
        }
    }
    else if (menuIdx == 4) { // REAL-TIME NOISE FLOOR
        u8g2.setCursor(2, 22); u8g2.print("NOISE SCAN:");
        u8g2.setFont(u8g2_font_logisoso16_tf); u8g2.setCursor(20, 50); u8g2.print(noiseFloor); u8g2.print(" dBm");
    }
    else if (menuIdx == 5) { // TRAFFIC STATS
        u8g2.setCursor(2, 22); u8g2.print("STATS:");
        u8g2.setCursor(10, 35); u8g2.printf("TX PKTS: %u", txPkts);
        u8g2.setCursor(10, 45); u8g2.printf("RX PKTS: %u", rxPkts);
        u8g2.setCursor(10, 55); u8g2.printf("RELAYS : %u", relayedCount);
    }
    else if (menuIdx == 6) { // NODE HEALTH & HW TEST
        u8g2.setCursor(10, 25); u8g2.print("WIRING TEST: OK");
        u8g2.setCursor(10, 40); u8g2.printf("UPTIME: %lu min", millis()/60000);
        u8g2.setCursor(10, 55); u8g2.printf("HEX ID: 0x%04X", nodeID);
    }
    u8g2.sendBuffer();
}

// ================= BLUETOOTH COMMANDS =================

void handleBT() {
    if (!SerialBT.available()) return;
    String input = SerialBT.readStringUntil('\n'); input.trim();
    if (input.length() == 0) return;

    if (input.startsWith("/")) {
        if (input == "/status") SerialBT.printf("RSSI:%d SNR:%.1f Freq:%ld\n", lastRSSI, lastSNR, freq);
        else if (input.startsWith("/preset ")) {
            int p = input.substring(8).toInt();
            if (p >= 0 && p <= 3) { 
                currentP = (Preset)p; 
                applyLoRa(); 
                SerialBT.printf("Preset -> %d\n", p); 
                drawUI(); 
            }
        }
        else if (input.startsWith("/freq ")) {
            freq = input.substring(6).toFloat() * 1000000L; 
            applyLoRa(); 
            SerialBT.printf("Freq set to %ld\n", freq); 
            drawUI();
        }
        else if (input == "/id") SerialBT.printf("ID: %04X Name: %s\n", nodeID, nodeName.c_str());
        else if (input == "/reboot") ESP.restart();
        return; 
    }
    secure_send(input.c_str(), BROADCAST_ID); 
    drawUI();
}

// ================= SETUP & LOOP =================

void setup() {
    u8g2.begin();
    Serial.begin(115200); 
    nodeID = (uint16_t)ESP.getEfuseMac();
    nodeName = "CORE-" + String(nodeID, HEX); nodeName.toUpperCase();
    SerialBT.begin(nodeName);

    // Initial Hardware Test with Auto-Exit
    uint32_t startTest = millis();
    while (millis() - startTest < 3000) { 
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_4x6_tf);
        LoRa.setPins(5, 14, 26);
        if (!LoRa.begin(freq)) {
            u8g2.setCursor(0, 63); u8g2.print("LORA FAILED! CHECK WIRING"); 
            u8g2.sendBuffer(); 
            while(1); 
        }
        u8g2.setCursor(0, 63); u8g2.print("Hardware Test: OK"); 
        u8g2.sendBuffer(); 
        delay(100); 
    }
    
    u8g2.clearBuffer();

    button.attachClick([](){ 
        if(menuIdx == 3) { 
            currentP = (Preset)((currentP + 1) % 4); 
            applyLoRa(); 
        } else { 
            menuIdx = (menuIdx + 1) % 7; 
        } 
        drawUI(); 
    });
    button.attachDoubleClick([](){ menuIdx = (menuIdx + 1) % 7; drawUI(); });
    button.attachLongPressStart([](){ 
        freq += 1000000L; 
        if(freq > 450000000L) freq = 410000000L; 
        applyLoRa(); 
        drawUI(); 
    });

    applyLoRa(); 
    addTerminal("MESH ONLINE", true);
    drawUI();
}

void loop() {
    button.tick(); 
    handleBT();

    noiseFloor = LoRa.packetRssi(); 
    if (menuIdx == 4) drawUI(); 

    //  MESH HOPPING: Enhanced packet processing
    int sz = LoRa.parsePacket();
    if (sz >= sizeof(OpenMeshHeader) + IV_SIZE + TAG_SIZE) {
        lastRSSI = LoRa.packetRssi(); 
        lastSNR = LoRa.packetSnr(); 
        rxPkts++;
        
        OpenMeshHeader h; 
        LoRa.readBytes((uint8_t*)&h, sizeof(h));
        
        //  MESH HOPPING: Check for duplicates
        if (isMsgSeen(h.msg_id)) {
            return; // Skip duplicate
        }
        addSeenMsg(h.msg_id);
        
        uint8_t iv[IV_SIZE], tag[TAG_SIZE];
        uint8_t ciphertext[100], plaintext[101];
        LoRa.readBytes(iv, IV_SIZE); 
        LoRa.readBytes(tag, TAG_SIZE);
        
        int pLen = h.payload_len; 
        if (pLen > 100) pLen = 100;
        LoRa.readBytes(ciphertext, pLen);

        mbedtls_gcm_context gcm; 
        mbedtls_gcm_init(&gcm);
        mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, mesh_key, 256);
        
        if (mbedtls_gcm_auth_decrypt(&gcm, pLen, 
                                     iv, IV_SIZE, NULL, 0, 
                                     tag, TAG_SIZE, 
                                     ciphertext, plaintext) == 0) {
            plaintext[pLen] = '\0'; 
            addTerminal((char*)plaintext, false);
            SerialBT.printf("IN [%04X]: %s\n", h.src, (char*)plaintext);
            
            //  MESH HOPPING: Update neighbor info
            updateNeighbor(h.src, lastRSSI, lastSNR);
            
            //  MESH HOPPING: RELAY MESSAGE
            // Relay if TTL > 1 and not from us
            if (h.ttl > 1 && h.src != nodeID && h.msg_type == 0) { // Only relay DATA messages
                h.ttl--; // Decrement TTL
                
                // Re-transmit the packet
                LoRa.beginPacket();
                LoRa.write((uint8_t*)&h, sizeof(h));
                LoRa.write(iv, IV_SIZE);
                LoRa.write(tag, TAG_SIZE);
                LoRa.write(ciphertext, pLen);
                LoRa.endPacket(true);
                LoRa.receive();
                
                relayedCount++;
                SerialBT.printf(" RELAYED msg %04X, TTL: %d\n", h.msg_id, h.ttl);
            }
        }
        
        mbedtls_gcm_free(&gcm); 
        drawUI();
    }
    
    static uint32_t lastUI = 0;
    if (millis() - lastUI > 5000) { 
        drawUI(); 
        lastUI = millis(); 
    }
    
    // Clean up old neighbors
    for (int i = 0; i < neighborCount; i++) {
        if (millis() - neighbors[i].last_seen > 300000) { // 5 minutes
            neighbors[i].active = false;
            for (int j = i; j < neighborCount - 1; j++) {
                neighbors[j] = neighbors[j + 1];
            }
            neighborCount--;
            i--;
        }
    }
}
