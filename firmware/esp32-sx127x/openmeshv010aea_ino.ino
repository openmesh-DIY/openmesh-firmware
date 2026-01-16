#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <OneButton.h>
#include "mbedtls/aes.h"
#include <Preferences.h>
#include "BluetoothSerial.h"

// ================= SECURITY =================
unsigned char aes_key[32] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,
  0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
  0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20
};
unsigned char static_iv[16] = {0};

// ================= PINS =================
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

// ================= NODE =================
uint16_t nodeID;
uint16_t lastRelayID = 0;

// ================= UI WORDS =================
const char* rows[4][4] = {
  {"WYA","OTW","MOVIN","SUP"},
  {"GOOD","BUSY","ONLINE","IDLE"},
  {"YES","NO","OK","WAIT"},
  {"NODE","LINK","HELP","COME"}
};

int rowIdx = 0;
int wordIdx = 0;

// ================= UI STATE =================
enum UIState {
  UI_WORD,
  UI_DEST
};

UIState uiState = UI_WORD;

// ================= DEST SELECT =================
uint16_t knownNodes[6] = {0}; // learned from packets
int knownCount = 0;
int destIndex = 0; // last index = broadcast

#define BROADCAST_ID 0xFFFF
#define MAX_TTL 8

// ================= AES SEND =================
void encrypt_and_send(const char* msg, uint16_t dest, uint8_t ttl = MAX_TTL) {
  unsigned char iv[16], input[16]={0}, output[16]={0};
  memcpy(iv, static_iv, 16);
  strncpy((char*)input, msg, 15);

  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, aes_key, 256);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, 16, iv, input, output);
  mbedtls_aes_free(&aes);

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&nodeID,2);
  LoRa.write((uint8_t*)&dest,2);
  LoRa.write(ttl);
  LoRa.write(iv,16);
  LoRa.write(output,16);
  LoRa.endPacket();

  SerialBT.printf("TX SRC:%u DEST:%u MSG:%s\n",nodeID,dest,msg);
}

// ================= UI =================
void drawWordUI() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.setCursor(0,10);
  u8g2.printf("Node:%u",nodeID);

  for(int i=0;i<4;i++){
    u8g2.setCursor(0,24+i*10);
    u8g2.print(i==wordIdx?"> ":"  ");
    u8g2.print(rows[rowIdx][i]);
  }
  u8g2.sendBuffer();
}

void drawDestUI() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.setCursor(0,10);
  u8g2.print("Send to:");

  for(int i=0;i<knownCount && i<3;i++){
    u8g2.setCursor(0,24+i*10);
    u8g2.print(destIndex==i?"> ":"  ");
    u8g2.printf("%u",knownNodes[i]);
  }

  u8g2.setCursor(0,24+knownCount*10);
  u8g2.print(destIndex==knownCount?"> ":"  ");
  u8g2.print("ALL");

  u8g2.sendBuffer();
}

// ================= BUTTONS =================
void singleClick(){
  if(uiState==UI_WORD){
    wordIdx=(wordIdx+1)%4;
    drawWordUI();
  } else {
    destIndex=(destIndex+1)%(knownCount+1);
    drawDestUI();
  }
}

void doubleClick(){
  if(uiState==UI_WORD){
    rowIdx=(rowIdx+1)%4;
    wordIdx=0;
    drawWordUI();
  }
}

void longPress(){
  if(uiState==UI_WORD){
    uiState=UI_DEST;
    destIndex=0;
    drawDestUI();
  } else {
    uint16_t dest = (destIndex==knownCount)?BROADCAST_ID:knownNodes[destIndex];
    encrypt_and_send(rows[rowIdx][wordIdx],dest);
    uiState=UI_WORD;
    drawWordUI();
  }
}

// ================= SETUP =================
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
  LoRa.begin(433E6);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(62.5E3);
  LoRa.setCodingRate4(5);
  LoRa.setTxPower(20);

  drawWordUI();
}

// ================= LOOP =================
void loop(){
  button.tick();

  int size=LoRa.parsePacket();
  if(size>=22){
    uint8_t buf[64]; int i=0;
    while(LoRa.available()) buf[i++]=LoRa.read();

    uint16_t src,dst; uint8_t ttl;
    memcpy(&src,buf,2);
    memcpy(&dst,buf+2,2);
    ttl=buf[4];

    if(src!=nodeID){
      bool known=false;
      for(int k=0;k<knownCount;k++) if(knownNodes[k]==src) known=true;
      if(!known && knownCount<6) knownNodes[knownCount++]=src;
    }
  }
}
