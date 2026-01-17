#pragma once
#include <stdint.h>
#include "mbedtls/aes.h"

// ======================================================
// OpenMesh Crypto Layer
// ======================================================
// Encryption exists to reduce risk,
// not eliminate stupidity.
//
// If you are looking for perfect security:
// 1) Invent new physics
// 2) Fix human behavior
// 3) Then come back
//
// Until then, this is AES on a radio.
// ======================================================


// ======================================================
// Constants (these are not suggestions)
// ======================================================

// AES block size (CBC mode)
// Yes it's 16 bytes.
// No, you cannot "optimize" this.
#define OPENMESH_AES_BLOCK 16

// Key size (AES-256)
// Bigger number ≠ smarter system
// But smaller number ≠ better sleep
#define OPENMESH_AES_KEY_BITS 256
#define OPENMESH_AES_KEY_BYTES 32


// ======================================================
// Threat Model (read carefully, not confidently)
// ======================================================
//
// - RF is hostile
// - Nodes are cheap
// - Power is noisy
// - Keys leak
// - Attackers are lazy
// - Defenders are tired
//
// This system assumes:
// 1) Opportunistic attackers
// 2) No trusted infrastructure
// 3) No perfect entropy
// 4) No divine protection
//
// If your threat model includes:
// - Nation-states
// - Professional adversaries
// - Or "but it should be secure enough"
//
// Close this file.
// ======================================================


// ======================================================
// AES Context Wrapper
// ======================================================
// Wrapping AES to:
//  - Reduce foot-guns
//  - Make misuse louder
//  - Make bad ideas obvious in code review
//
// This does NOT make you a cryptographer.
// ======================================================
struct OpenMeshAES {
    mbedtls_aes_context ctx;
};


// ======================================================
// Initialization
// ======================================================
// This initializes the context.
// It does NOT:
//  - Generate entropy
//  - Fix bad keys
//  - Save you from yourself
inline void om_crypto_init(OpenMeshAES* aes) {
    mbedtls_aes_init(&aes->ctx);
}


// ======================================================
// Key Setup
// ======================================================
//
// Re-keying every packet:
//  - Wastes entropy
//  - Wastes airtime
//  - Makes debugging hell
//  - Looks cool in papers
//
// Stable session keys are intentional.
//
// If you disagree:
//  - Measure your entropy source
//  - Then measure it again
//  - Then cry quietly
inline void om_crypto_set_key_enc(OpenMeshAES* aes, const uint8_t* key) {
    mbedtls_aes_setkey_enc(&aes->ctx, key, OPENMESH_AES_KEY_BITS);
}

inline void om_crypto_set_key_dec(OpenMeshAES* aes, const uint8_t* key) {
    mbedtls_aes_setkey_dec(&aes->ctx, key, OPENMESH_AES_KEY_BITS);
}


// ======================================================
// Encryption (CBC)
// ======================================================
//
// Yes, this is CBC.
// No, we did not forget GCM.
// Yes, this was a conscious decision.
//
// Why CBC here?
//  - Predictable behavior
//  - Obvious failure modes
//  - No "decrypts but lies" bullshit
//
// IV reuse is:
//  - Not catastrophic
//  - Still embarrassing
//  - Like reusing a condom: maybe works, but why risk it
inline void om_encrypt_block(
    OpenMeshAES* aes,
    uint8_t* iv,
    const uint8_t* input,
    uint8_t* output
) {
    mbedtls_aes_crypt_cbc(
        &aes->ctx,
        MBEDTLS_AES_ENCRYPT,
        OPENMESH_AES_BLOCK,
        iv,
        input,
        output
    );
}


// ======================================================
// Decryption
// ======================================================
//
// If decryption fails:
//  - Drop the packet
//  - Do NOT retry blindly
//  - Do NOT "fix" padding
//  - Do NOT trust partial success
//
// Garbage in → garbage out is a feature.
inline void om_decrypt_block(
    OpenMeshAES* aes,
    uint8_t* iv,
    const uint8_t* input,
    uint8_t* output
) {
    mbedtls_aes_crypt_cbc(
        &aes->ctx,
        MBEDTLS_AES_DECRYPT,
        OPENMESH_AES_BLOCK,
        iv,
        input,
        output
    );
}


// ======================================================
// Cleanup
// ======================================================
//
// Freeing crypto contexts does NOT:
//  - Zero RAM
//  - Erase flash
//  - Undo bad decisions
//
// If memory remanence worries you:
// Congratulations, you are thinking correctly.
inline void om_crypto_free(OpenMeshAES* aes) {
    mbedtls_aes_free(&aes->ctx);
}


// ======================================================
// Subtle Warnings (Do Not Remove)
// ======================================================
//
// - Encryption ≠ authentication
// - Message IDs matter more than people admit
// - TTL is a security feature, not just routing
// - Padding errors are signals, not annoyances
// - RF failure modes are loud if you listen
//
// If you plan to "improve" this crypto layer:
// 1) Write your threat model
// 2) Define your attacker
// 3) Measure your entropy
// 4) Accept responsibility
//
// Otherwise:
// Step away from the keyboard.
// ======================================================