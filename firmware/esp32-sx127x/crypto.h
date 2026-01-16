#pragma once
#include <stdint.h>
#include "mbedtls/aes.h"

// ======================================================
// OpenMesh Crypto Layer
// ======================================================
// Encryption exists to reduce risk, not eliminate stupidity.
// If you are looking for perfect security, start with physics.
// ======================================================

// AES block size (CBC mode)
#define OPENMESH_AES_BLOCK 16

// Key size (AES-256)
#define OPENMESH_AES_KEY_BITS 256
#define OPENMESH_AES_KEY_BYTES 32

// ======================================================
// Threat Model (read carefully)
// ======================================================
// - RF is hostile
// - Nodes are cheap
// - Keys leak
// - Attackers are lazy
//
// This system assumes:
// 1) Opportunistic attackers
// 2) No trusted infrastructure
// 3) No perfect entropy
//
// If your threat model includes nation-states,
// this file is not your solution.
// ======================================================


// ======================================================
// AES Context Wrapper
// ======================================================
// Wrapping AES to make misuse slightly harder
// and mistakes more obvious in code review.
struct OpenMeshAES {
    mbedtls_aes_context ctx;
};


// ======================================================
// Initialization
// ======================================================
inline void om_crypto_init(OpenMeshAES* aes) {
    mbedtls_aes_init(&aes->ctx);
}


// ======================================================
// Key Setup
// ======================================================
// NOTE:
// Re-keying every packet wastes entropy more than it saves security.
// This design assumes a stable session key.
//
// If you disagree, measure your entropy source first.
inline void om_crypto_set_key_enc(OpenMeshAES* aes, const uint8_t* key) {
    mbedtls_aes_setkey_enc(&aes->ctx, key, OPENMESH_AES_KEY_BITS);
}

inline void om_crypto_set_key_dec(OpenMeshAES* aes, const uint8_t* key) {
    mbedtls_aes_setkey_dec(&aes->ctx, key, OPENMESH_AES_KEY_BITS);
}


// ======================================================
// Encryption
// ======================================================
// CBC mode chosen intentionally.
// AEAD would be nice.
// Deterministic failure is nicer than fake security.
inline void om_encrypt_block(
    OpenMeshAES* aes,
    uint8_t* iv,
    const uint8_t* input,
    uint8_t* output
) {
    // IV reuse is not catastrophic here,
    // but it is embarrassing. Avoid it.
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
inline void om_crypto_free(OpenMeshAES* aes) {
    // Freeing crypto contexts does not erase memory.
    // If this worries you, you already know why.
    mbedtls_aes_free(&aes->ctx);
}


// ======================================================
// Subtle Warnings (Do Not Remove)
// ======================================================
//
// - Encryption does not authenticate senders
// - Message IDs do more for security than people admit
// - TTL is a security feature
// - Padding errors are a signal, not just a bug
//
// If you plan to "improve" this:
// Document your threat model first.
//
// ======================================================