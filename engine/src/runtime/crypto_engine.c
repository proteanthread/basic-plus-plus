// FILENAME: crypto_engine.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (func_crypto.c)
// NEEDS: libcore (crypto_engine.h, memops.h, memops.c, strops.h, strops.c)
// Implements freestanding C17 cryptography functions: SHA-256, HMAC, Keygen, and Stream Encryption.
//
// ---- Includes ----

#include "runtime/crypto_engine.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include <stdio.h>

// SHA-256 implementation
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void crypto_sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };
    uint8_t block[64];
    size_t bitlen = len * 8;
    size_t i = 0;

    while (i < len) {
        size_t chunk = (len - i < 64) ? (len - i) : 64;
        runtime_memcpy(block, data + i, chunk);
        if (chunk < 64) {
            block[chunk++] = 0x80;
            if (chunk > 56) {
                while (chunk < 64) block[chunk++] = 0x00;
                // Process block
                uint32_t w[64];
                for (int t = 0; t < 16; ++t) {
                    w[t] = ((uint32_t)block[t * 4] << 24) | ((uint32_t)block[t * 4 + 1] << 16) |
                           ((uint32_t)block[t * 4 + 2] << 8) | ((uint32_t)block[t * 4 + 3]);
                }
                for (int t = 16; t < 64; ++t) w[t] = SIG1(w[t-2]) + w[t-7] + SIG0(w[t-15]) + w[t-16];
                uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], f = state[5], g = state[6], h = state[7];
                for (int t = 0; t < 64; ++t) {
                    uint32_t t1 = h + EP1(e) + CH(e, f, g) + k[t] + w[t];
                    uint32_t t2 = EP0(a) + MAJ(a, b, c);
                    h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
                }
                state[0] += a; state[1] += b; state[2] += c; state[3] += d;
                state[4] += e; state[5] += f; state[6] += g; state[7] += h;
                chunk = 0;
            }
            while (chunk < 56) block[chunk++] = 0x00;
            for (int t = 0; t < 8; ++t) block[63 - t] = (uint8_t)(bitlen >> (t * 8));
        }

        uint32_t w[64];
        for (int t = 0; t < 16; ++t) {
            w[t] = ((uint32_t)block[t * 4] << 24) | ((uint32_t)block[t * 4 + 1] << 16) |
                   ((uint32_t)block[t * 4 + 2] << 8) | ((uint32_t)block[t * 4 + 3]);
        }
        for (int t = 16; t < 64; ++t) w[t] = SIG1(w[t-2]) + w[t-7] + SIG0(w[t-15]) + w[t-16];
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], f = state[5], g = state[6], h = state[7];
        for (int t = 0; t < 64; ++t) {
            uint32_t t1 = h + EP1(e) + CH(e, f, g) + k[t] + w[t];
            uint32_t t2 = EP0(a) + MAJ(a, b, c);
            h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;

        i += 64;
    }

    for (int t = 0; t < 8; ++t) {
        out[t * 4]     = (uint8_t)(state[t] >> 24);
        out[t * 4 + 1] = (uint8_t)(state[t] >> 16);
        out[t * 4 + 2] = (uint8_t)(state[t] >> 8);
        out[t * 4 + 3] = (uint8_t)(state[t]);
    }
}

void crypto_sha256_hex(const char *data, size_t len, char out_hex[65]) {
    uint8_t hash[32];
    crypto_sha256((const uint8_t *)data, len, hash);
    static const char hex_digits[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[i * 2]     = hex_digits[(hash[i] >> 4) & 0x0F];
        out_hex[i * 2 + 1] = hex_digits[hash[i] & 0x0F];
    }
    out_hex[64] = '\0';
}

void crypto_hmac_sha256_hex(const char *key, size_t key_len, const char *data, size_t data_len, char out_hex[65]) {
    uint8_t k_buf[64] = {0};
    if (key_len > 64) {
        crypto_sha256((const uint8_t *)key, key_len, k_buf);
    } else {
        runtime_memcpy(k_buf, key, key_len);
    }

    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    for (int i = 0; i < 64; ++i) {
        k_ipad[i] = k_buf[i] ^ 0x36;
        k_opad[i] = k_buf[i] ^ 0x5c;
    }

    // Hash inner
    uint8_t inner[64 + 1024];
    size_t in_len = 64 + data_len;
    if (in_len > sizeof(inner)) in_len = sizeof(inner);
    runtime_memcpy(inner, k_ipad, 64);
    if (data_len > 0) runtime_memcpy(inner + 64, data, in_len - 64);
    uint8_t inner_hash[32];
    crypto_sha256(inner, in_len, inner_hash);

    // Hash outer
    uint8_t outer[64 + 32];
    runtime_memcpy(outer, k_opad, 64);
    runtime_memcpy(outer + 64, inner_hash, 32);
    uint8_t out_hash[32];
    crypto_sha256(outer, sizeof(outer), out_hash);

    static const char hex_digits[] = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out_hex[i * 2]     = hex_digits[(out_hash[i] >> 4) & 0x0F];
        out_hex[i * 2 + 1] = hex_digits[out_hash[i] & 0x0F];
    }
    out_hex[64] = '\0';
}

void crypto_encrypt_sim(const char *key, const char *plain, size_t len, char *out_hex, size_t out_max) {
    if (!key || !plain || !out_hex || out_max == 0) return;
    size_t klen = runtime_strlen(key);
    if (klen == 0) {
        klen = 1;
        key = "K";
    }
    static const char hex_digits[] = "0123456789abcdef";
    size_t out_pos = 0;
    for (size_t i = 0; i < len && out_pos + 2 < out_max; ++i) {
        uint8_t c = (uint8_t)plain[i] ^ (uint8_t)key[i % klen];
        out_hex[out_pos++] = hex_digits[(c >> 4) & 0x0F];
        out_hex[out_pos++] = hex_digits[c & 0x0F];
    }
    out_hex[out_pos] = '\0';
}

void crypto_decrypt_sim(const char *key, const char *hex_cipher, size_t len, char *out_plain, size_t out_max) {
    if (!key || !hex_cipher || !out_plain || out_max == 0) return;
    size_t klen = runtime_strlen(key);
    if (klen == 0) {
        klen = 1;
        key = "K";
    }
    size_t out_pos = 0;
    for (size_t i = 0; i + 1 < len && out_pos + 1 < out_max; i += 2) {
        char h1 = hex_cipher[i];
        char h2 = hex_cipher[i + 1];
        uint8_t v1 = (h1 >= '0' && h1 <= '9') ? (h1 - '0') : ((h1 >= 'a' && h1 <= 'f') ? (h1 - 'a' + 10) : (h1 - 'A' + 10));
        uint8_t v2 = (h2 >= '0' && h2 <= '9') ? (h2 - '0') : ((h2 >= 'a' && h2 <= 'f') ? (h2 - 'a' + 10) : (h2 - 'A' + 10));
        uint8_t c = ((v1 << 4) | v2) ^ (uint8_t)key[(out_pos) % klen];
        out_plain[out_pos++] = (char)c;
    }
    out_plain[out_pos] = '\0';
}

void crypto_keygen(int bits, char *out_hex, size_t out_max) {
    if (!out_hex || out_max == 0) return;
    int bytes = (bits > 0) ? (bits / 8) : 16;
    if (bytes < 16) bytes = 16;
    if (bytes > 64) bytes = 64;
    static const char hex_digits[] = "0123456789abcdef";
    size_t out_pos = 0;
    for (int i = 0; i < bytes && out_pos + 2 < out_max; ++i) {
        uint8_t r = (uint8_t)((i * 37 + 101) ^ (bytes * 13));
        out_hex[out_pos++] = hex_digits[(r >> 4) & 0x0F];
        out_hex[out_pos++] = hex_digits[r & 0x0F];
    }
    out_hex[out_pos] = '\0';
}
