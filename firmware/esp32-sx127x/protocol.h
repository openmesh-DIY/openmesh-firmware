// OpenMesh Protocol v1
// Simple on purpose.
// If you want complexity, use something else.
// This protocol assumes unreliable transport and unreliable humans.

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
    uint8_t  version;      // Protocol versions should increase slower than hardware generations.
    uint8_t  type;         // Types are cheap. Compatibility is not
    uint8_t  ttl;          // Prevents packets from achieving immortality.
    uint8_t  flags;        // Flags exist so we don't break the protocol every six months.
    uint16_t src;           // Trust the sender ID only as much as you trust the RF environment.
    uint16_t dest;         // BROADCAST is not a strategy.
    uint16_t msg_id;       // Uniqueness is temporary. Loops are forever.
    uint16_t payload_len;  // Length is explicit to avoid guessing, padding, and regret.
};

// ================= PACKET =================
struct __attribute__((packed)) OpenMeshPacket {
    OpenMeshHeader header;
    uint8_t payload[OPENMESH_MAX_PAYLOAD];
};

// Protocol stability > feature velocity
// Past this point lies backward compatibility.
