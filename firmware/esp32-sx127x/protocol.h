#pragma once
#include <stdint.h>

#define OPENMESH_PROTOCOL_VERSION 1

// ================= LIMITS =================
#define OPENMESH_MAX_PAYLOAD     180
#define OPENMESH_BROADCAST_ID    0xFFFF   // match firmware (uint16_t)
#define OPENMESH_MAX_TTL         8

// ================= PACKET TYPES =================
enum OpenMeshPacketType : uint8_t {
    OM_PKT_TEXT      = 0x01,   // chat / quick messages
    OM_PKT_ACK       = 0x02,   // optional delivery ack
    OM_PKT_HELLO     = 0x03,   // node discovery
    OM_PKT_NODEINFO  = 0x04,   // node info response
    OM_PKT_CONFIG    = 0x05    // config over BT / LoRa
};

// ================= FLAGS =================
#define OM_FLAG_ENCRYPTED  0x01
#define OM_FLAG_RELAYED    0x02
#define OM_FLAG_ACK_REQ    0x04

// ================= HEADER =================
struct __attribute__((packed)) OpenMeshHeader {
    uint8_t  version;      // protocol version (1)
    uint8_t  type;         // OpenMeshPacketType
    uint8_t  ttl;          // hop counter
    uint8_t  flags;        // OM_FLAG_*
    uint16_t src;          // source node ID
    uint16_t dest;         // dest node ID / broadcast
    uint16_t msg_id;       // anti-loop + ack tracking
    uint16_t payload_len;  // bytes after header
};

// ================= PACKET =================
struct __attribute__((packed)) OpenMeshPacket {
    OpenMeshHeader header;
    uint8_t payload[OPENMESH_MAX_PAYLOAD];
};
