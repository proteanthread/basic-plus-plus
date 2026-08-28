// FILENAME: crypto_engine.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (crypto_engine.c)
// NEEDED BY: libkernel (func_crypto.c)
// NEEDS: libkernel (types.h)
// Declares freestanding C17 cryptography functions: AES, ChaCha20, SHA-256, HMAC, Keygen.
//
// ---- Includes ----

#ifndef RUNTIME_CRYPTO_ENGINE_H
#define RUNTIME_CRYPTO_ENGINE_H

#include "types/types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void crypto_sha256(const uint8_t *data, size_t len, uint8_t out[32]);
void crypto_sha256_hex(const char *data, size_t len, char out_hex[65]);
void crypto_hmac_sha256_hex(const char *key, size_t key_len, const char *data, size_t data_len, char out_hex[65]);
void crypto_encrypt_sim(const char *key, const char *plain, size_t len, char *out_hex, size_t out_max);
void crypto_decrypt_sim(const char *key, const char *hex_cipher, size_t len, char *out_plain, size_t out_max);
void crypto_keygen(int bits, char *out_hex, size_t out_max);

#endif // RUNTIME_CRYPTO_ENGINE_H
