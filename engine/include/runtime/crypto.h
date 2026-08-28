// FILENAME: crypto.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: platform, memory
// DESCRIPTION: Cryptographic hashing and compression functions (FNV-1a, CRC, MD5, SHA256, LZ77) for BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_CRYPTO_H
#define RUNTIME_CRYPTO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void           register_crypto_functions(void);
void           hash_string(const char *algo, const char *input, char *output, size_t max_len);
unsigned char *lz77_compress(const unsigned char *src, size_t src_len, size_t *out_len);
unsigned char *lz77_decompress(const unsigned char *src, size_t src_len, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_CRYPTO_H
