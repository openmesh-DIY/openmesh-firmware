#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/aes.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= PROTOCOL DEFINITIONS =================
#define OPENMESH_MAX_PAYLOAD 180
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

// ================= CONFIG & SECURITY =================
unsigned char aes_key[32] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
};

#define BUTTON_PIN 13
#define OLED_SDA 21
#define OLED_SCL 22
#define LORA_SCK  18
#define LORA_MISO 19
#define LORA_MOSI 23
#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 26

// ================= OBJECTS =================
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
OneButton button(BUTTON_PIN, true);
BluetoothSerial SerialBT;
Preferences prefs;

// ================= STATE =================
uint16_t nodeID;
uint16_t lastMsgID = 0;
uint16_t seenMsgIDs[10] = {0}; // Simple anti-loop buffer
int seenIdx = 0;

uint16_t knownNodes[6] = {0};
int knownCount = 0;
int destIndex = 0; 

const char* rows[4][4] = {
  {"WYA","OTW","MOVIN","SUP"},
  {"GOOD","BUSY","ONLINE","IDLE"},
  {"YES","NO","OK","WAIT"},
  {"NODE","LINK","HELP","COME"}
};
int rowIdx = 0; int wordIdx = 0;
enum UIState { UI_WORD, UI_DEST };
UIState uiState = UI_WORD;

// ================= MESH LOGIC =================

void encrypt_and_send(const char* msg, uint16_t dest) {
  lastMsgID++;
  OpenMeshHeader header = {1, 0x01, MAX_TTL, 0x01, nodeID, dest, lastMsgID, 16};
  
  unsigned char iv[16];
  for(int i=0; i<16; i++) iv[i] = (uint8_t)random(0, 255); // Random IV per packet
  
  unsigned char input[16] = {0};
  unsigned char output[16] = {0};
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
  
  SerialBT.printf("TX: %s to %u\n", msg, dest);
}

void relay_packet(OpenMeshHeader* h, uint8_t* iv, uint8_t* encrypted_payload) {
  if (h->ttl <= 1) return; // Don't relay if TTL spent
  h->ttl--;
  
  // Small random delay to prevent LoRa collisions
  delay(random(100, 500)); 

  LoRa.beginPacket();
  LoRa.write((uint8_t*)h, sizeof(OpenMeshHeader));
  LoRa.write(iv, 16);
  LoRa.write(encrypted_payload, 16);
  LoRa.endPacket();
  Serial.printf("Relayed MsgID: %u\n", h->msg_id);
}

// ================= UI FUNCTIONS =================
void drawWordUI() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.setCursor(0,10); u8g2.printf("ID:%u | Mesh Ready", nodeID);
  for(int i=0;i<4;i++){
    u8g2.setCursor(0,24+i*10);
    u8g2.print(i==wordIdx?"> ":"  ");
    u8g2.print(rows[rowIdx][i]);
  }
  u8g2.sendBuffer();
}

void drawDestUI() {
  u8g2.clearBuffer();
  u8g2.setCursor(0,10); u8g2.print("Target:");
  for(int i=0;i<knownCount;i++){
    u8g2.setCursor(0,24+i*10);
    u8g2.print(destIndex==i?"> ":"  ");
    u8g2.printf("Node %u", knownNodes[i]);
  }
  u8g2.setCursor(0,24+knownCount*10);
  u8g2.print(destIndex==knownCount?"> ":"  ");
  u8g2.print("ALL (Broadcast)");
  u8g2.sendBuffer();
}

// ================= BUTTON HANDLERS =================
void singleClick(){
  if(uiState==UI_WORD) { wordIdx=(wordIdx+1)%4; drawWordUI(); }
  else { destIndex=(destIndex+1)%(knownCount+1); drawDestUI(); }
}
void doubleClick(){
  if(uiState==UI_WORD) { rowIdx=(rowIdx+1)%4; wordIdx=0; drawWordUI(); }
}
void longPress(){
  if(uiState==UI_WORD){ uiState=UI_DEST; drawDestUI(); }
  else {
    uint16_t dest = (destIndex==knownCount)?BROADCAST_ID:knownNodes[destIndex];
    encrypt_and_send(rows[rowIdx][wordIdx], dest);
    uiState=UI_WORD; drawWordUI();
  }
}

// ================= CORE =================
void setup(){
  Serial.begin(115200);
  SerialBT.begin("OpenMesh_Node");
  
  prefs.begin("node",false);
  nodeID=prefs.getUShort("id",0);
  if(nodeID==0){ nodeID=random(1,65535); prefs.putUShort("id",nodeID); }

  button.attachClick(singleClick);
  button.attachDoubleClick(doubleClick);
  button.attachLongPressStart(longPress);

  u8g2.begin();
  SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_SS);
  LoRa.setPins(LORA_SS,LORA_RST,LORA_DIO0);
  if(!LoRa.begin(433E6)) { Serial.println("LoRa Fail"); while(1); }
  
  LoRa.setSpreadingFactor(12); // Max range
  LoRa.setSignalBandwidth(62.5E3);
  drawWordUI();
}

void loop(){
  button.tick();

  int packetSize = LoRa.parsePacket();
  if (packetSize >= sizeof(OpenMeshHeader) + 32) {
    OpenMeshHeader h;
    LoRa.readBytes((uint8_t*)&h, sizeof(h));
    
    uint8_t iv[16]; uint8_t encrypted[16];
    LoRa.readBytes(iv, 16);
    LoRa.readBytes(encrypted, 16);

    // 1. Anti-Loop: Have we seen this message ID from this sender?
    bool seen = false;
    for(int i=0; i<10; i++) if(seenMsgIDs[i] == h.msg_id) seen = true;
    if(seen || h.src == nodeID) return; 
    seenMsgIDs[seenIdx] = h.msg_id; seenIdx = (seenIdx + 1) % 10;

    // 2. Discover Node
    bool known = false;
    for(int k=0; k<knownCount; k++) if(knownNodes[k] == h.src) known = true;
    if(!known && knownCount < 6) knownNodes[knownCount++] = h.src;

    // 3. Decrypt if for us or broadcast
    if(h.dest == nodeID || h.dest == BROADCAST_ID) {
      unsigned char decrypted[16] = {0};
      mbedtls_aes_context aes;
      mbedtls_aes_init(&aes);
      mbedtls_aes_setkey_dec(&aes, aes_key, 256);
      mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, 16, iv, encrypted, decrypted);
      mbedtls_aes_free(&aes);
      
      Serial.printf("RX from %u: %s\n", h.src, decrypted);
      SerialBT.printf("MSG [%u]: %s\n", h.src, decrypted);
    }

    // 4. Relay
    if(h.dest != nodeID) {
      relay_packet(&h, iv, encrypted);
    }
  }
}
