// ======================================================
// OpenMesh Protocol v1
// ======================================================
// Simple on purpose.
// If you want complexity, use something else.
// This protocol assumes:
//  - Unreliable transport
//  - Unreliable humans
//  - Unreliable power
//
// The only reliable thing here is disappointment
// if you overthink it.
// ======================================================

#pragma once
#include <stdint.h>

#define OPENMESH_PROTOCOL_VERSION 1
// Version bumps are expensive.
// If you increment this casually, you are the problem.


// ================= LIMITS =================
//
// These limits exist because:
//  - RAM is finite
//  - Airtime is expensive
//  - Radios are not magic
//
#define OPENMESH_MAX_PAYLOAD     180
// If you need more than this:
//  - Split the message
//  - Or write a book
//  - Or use the internet

#define OPENMESH_BROADCAST_ID    0xFFFF   // match firmware (uint16_t)
// Broadcast is a tool.
// Broadcast is NOT a lifestyle choice.

#define OPENMESH_MAX_TTL         8
// TTL prevents packets from achieving:
//  - Immortality
//  - Enlightenment
//  - Network-wide annoyance


// ================= PACKET TYPES =================
//
// Packet types are explicit.
// Overloading meanings leads to pain, hacks, and forks.
//
enum OpenMeshPacketType : uint8_t {
    OM_PKT_TEXT      = 0x01,   // Chat / quick messages / "WYA"
    OM_PKT_ACK       = 0x02,   // Optional delivery acknowledgement (best-effort, like life)
    OM_PKT_HELLO     = 0x03,   // Node discovery / "I exist, please notice me"
    OM_PKT_NODEINFO  = 0x04,   // Node info response / "This is who I am"
    OM_PKT_CONFIG    = 0x05    // Config over BT / LoRa (dangerous if abused)
};

// If you add more types:
//  - Document them
//  - Version them
//  - Do NOT reuse numbers
// Numbers are cheap. Compatibility is not.


// ================= FLAGS =================
//
// Flags exist so we don't break the protocol
// every time someone has a new idea at 2 AM.
//
#define OM_FLAG_ENCRYPTED  0x01
// Payload is encrypted.
// Metadata is still visible.
// Welcome to radio.

#define OM_FLAG_RELAYED    0x02
// Packet was forwarded.
// No, this does not mean it was "trusted".

#define OM_FLAG_ACK_REQ    0x04
// Sender would *like* an ACK.
// The network makes no promises.


// ================= HEADER =================
//
// This header is packed on purpose.
// Do NOT reorder fields casually.
// Do NOT add fields without a version bump.
//
struct __attribute__((packed)) OpenMeshHeader {
    uint8_t  version;
    // Protocol versions should increase
    // slower than hardware generations.

    uint8_t  type;
    // Types are cheap.
    // Debugging broken compatibility is not.

    uint8_t  ttl;
    // Prevents packets from looping forever
    // like bad ideas on the internet.

    uint8_t  flags;
    // Flags are how we evolve
    // without torching everything.

    uint16_t src;
    // Trust the sender ID only as much
    // as you trust the RF environment.

    uint16_t dest;
    // BROADCAST is not a routing strategy.
    // It's a blunt instrument.

    uint16_t msg_id;
    // Uniqueness is temporary.
    // Loops are forever.

    uint16_t payload_len;
    // Explicit length avoids:
    //  - Guessing
    //  - Padding bugs
    //  - Silent corruption
    //  - Regret
};


// ================= PACKET =================
//
// Fixed-size packet container.
// Yes, this wastes some bytes.
// No, we do not care.
//
struct __attribute__((packed)) OpenMeshPacket {
    OpenMeshHeader header;
    uint8_t payload[OPENMESH_MAX_PAYLOAD];
};


// ======================================================
// Stability Contract
// ======================================================
//
// Protocol stability > feature velocity
//
// Past this point lies:
//  - Backward compatibility
//  - Old nodes
//  - Users who won't reflash
//
// Break this lightly and you deserve the bug reports.
// ======================================================