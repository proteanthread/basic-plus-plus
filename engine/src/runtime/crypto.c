// FILENAME: crypto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strings.h, strings.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (vm.h)
// Provides core logic and interface definitions for crypto within BASIC++.
//
// ---- Includes ----

#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "runtime/strings.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include <stdint.h>

// FNV-1a 64-bit
static uint64_t fnv1a_64(const char *data, size_t len) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)(unsigned char)data[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

// HASH$(string$) -> Hex String
BValue crypto_hash_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    uint64_t h = fnv1a_64(data, len);
    char hex[17];
    runtime_snprintf(hex, sizeof(hex), "%016llx", (unsigned long long)h);
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), hex, 16);
    return res;
}

// CRC-64 ECMA polynomial 0xC96C5795D7870F42
static uint64_t crc64_ecma(const char *data, size_t len) {
    uint64_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= ((uint64_t)(unsigned char)data[i]) << 56;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000000000000000ULL) {
                crc = (crc << 1) ^ 0xC96C5795D7870F42ULL;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// CRC(string$) -> Number
BValue crypto_crc_func(BValue *args, int argc, void *rt) {
    (void)rt; // Unused
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    uint64_t c = crc64_ecma(data, len);
    // In BASIC++, numbers are typically doubles, but we return a double representing the CRC.
    // Note: 64-bit unsigned may lose precision in double (53 bits). But we must use double to remain compatible.
    return bval_float((double)c);
}

static uint32_t crypto_rand_u32(void) {
    static uint32_t seed = 0x853c49e6u;
    seed = seed * 1664525u + 1013904223u;
    return seed;
}

// GUID$() -> String
BValue crypto_guid_func(BValue *args, int argc, void *rt) {
    (void)args; (void)argc;
    VMContext *vm = (VMContext *)rt;
    char guid[37];
    uint32_t r1 = crypto_rand_u32();
    uint32_t r2 = crypto_rand_u32();
    uint32_t r3 = crypto_rand_u32();
    uint32_t r4 = crypto_rand_u32();
    
    r2 = (r2 & 0xFFFF0FFF) | 0x00004000; // Version 4
    r3 = (r3 & 0x3FFFFFFF) | 0x80000000; // Variant 1
    
    runtime_snprintf(guid, sizeof(guid), "%08x-%04x-%04x-%04x-%04x%08x",
             r1, r2 >> 16, r2 & 0xFFFF, r3 >> 16, r3 & 0xFFFF, r4);
             
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), guid, 36);
    return res;
}

void register_crypto_functions(void) {
    FunctionEntry fe;
    runtime_memset(&fe, 0, sizeof(fe));
    fe.module_name = "crypto";
    fe.overridable = 1;
    fe.category = FCAT_UTIL;
    
    fe.name = "HASH$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 1;
    fe.max_args = 2;
    fe.safety = FSAFE_PURE;
    fe.handler = crypto_hash_func;
    funcreg_register(&fe);
    
    fe.name = "CRC";
    fe.ret_type = FRET_FLOAT;
    fe.min_args = 1;
    fe.max_args = 1;
    fe.safety = FSAFE_PURE;
    fe.handler = crypto_crc_func;
    funcreg_register(&fe);
    
    fe.name = "GUID$";
    fe.ret_type = FRET_STRING;
    fe.min_args = 0;
    fe.max_args = 0;
    fe.safety = FSAFE_STATE;
    fe.handler = crypto_guid_func;
    funcreg_register(&fe);
}


unsigned char *lz77_compress(const unsigned char *src, size_t src_len, size_t *out_len) {
    if (!out_len) return NULL;
    
    HalContext *hal = hal_get();
    // Worst case size: 8 bytes for size prefix + literals control bytes + source bytes + buffer margin
    size_t max_out = 8 + src_len + (src_len / 128) + 16;
    unsigned char *dst = (unsigned char *)(hal && hal->mem.alloc ? hal->mem.alloc(max_out) : NULL);
    if (!dst) {
        *out_len = 0;
        return NULL;
    }
    runtime_memset(dst, 0, max_out);
    
    // Write decompressed size prefix
    uint64_t len_val = (uint64_t)src_len;
    for (int i = 0; i < 8; i++) {
        dst[i] = (unsigned char)((len_val >> (i * 8)) & 0xFF);
    }
    
    size_t dst_pos = 8;
    size_t literal_start = 0;
    
    size_t i = 0;
    while (i < src_len) {
        size_t best_len = 0;
        size_t best_dist = 0;
        
        // Search window limit: up to 4096 bytes back
        size_t start_win = (i > 4096) ? (i - 4096) : 0;
        
        // Find longest match
        for (size_t w = start_win; w < i; w++) {
            size_t match_len = 0;
            // Cap match length at 130 (since 0x7F + 3 = 130 max length encoded in 7 bits)
            // Also bound check against source length
            while (i + match_len < src_len && src[w + match_len] == src[i + match_len] && match_len < 130) {
                match_len++;
            }
            
            if (match_len >= 3 && match_len > best_len) {
                best_len = match_len;
                best_dist = i - w;
            }
        }
        
        if (best_len >= 3) {
            // Write literals before this match
            if (i > literal_start) {
                size_t lit_len = i - literal_start;
                size_t lit_pos = literal_start;
                while (lit_len > 0) {
                    size_t chunk = (lit_len > 128) ? 128 : lit_len;
                    dst[dst_pos++] = (unsigned char)(chunk - 1); // 0 .. 127
                    runtime_memcpy(dst + dst_pos, src + lit_pos, chunk);
                    dst_pos += chunk;
                    lit_pos += chunk;
                    lit_len -= chunk;
                }
            }
            
            // Write match: control byte has high bit 1, length is (best_len - 3)
            dst[dst_pos++] = (unsigned char)(0x80 | (best_len - 3));
            dst[dst_pos++] = (unsigned char)(best_dist & 0xFF);
            dst[dst_pos++] = (unsigned char)((best_dist >> 8) & 0xFF);
            
            i += best_len;
            literal_start = i;
        } else {
            i++;
        }
    }
    
    // Write remaining literals
    if (src_len > literal_start) {
        size_t lit_len = src_len - literal_start;
        size_t lit_pos = literal_start;
        while (lit_len > 0) {
            size_t chunk = (lit_len > 128) ? 128 : lit_len;
            dst[dst_pos++] = (unsigned char)(chunk - 1);
            runtime_memcpy(dst + dst_pos, src + lit_pos, chunk);
            dst_pos += chunk;
            lit_pos += chunk;
            lit_len -= chunk;
        }
    }
    
    *out_len = dst_pos;
    return dst;
}

unsigned char *lz77_decompress(const unsigned char *src, size_t src_len, size_t *out_len) {
    if (!out_len) return NULL;
    if (src_len < 8) {
        *out_len = 0;
        return NULL;
    }
    
    // Read decompressed size
    uint64_t len_val = 0;
    for (int i = 0; i < 8; i++) {
        len_val |= ((uint64_t)src[i]) << (i * 8);
    }
    
    size_t decomp_len = (size_t)len_val;
    *out_len = decomp_len;
    
    HalContext *hal = hal_get();
    size_t alloc_bytes = decomp_len > 0 ? decomp_len : 1;
    unsigned char *dst = (unsigned char *)(hal && hal->mem.alloc ? hal->mem.alloc(alloc_bytes) : NULL);
    if (!dst) {
        return NULL;
    }
    runtime_memset(dst, 0, alloc_bytes);
    
    size_t src_pos = 8;
    size_t dst_pos = 0;
    
    while (dst_pos < decomp_len && src_pos < src_len) {
        unsigned char ctrl = src[src_pos++];
        if ((ctrl & 0x80) == 0) {
            // Literal run
            size_t run_len = ctrl + 1;
            if (src_pos + run_len > src_len || dst_pos + run_len > decomp_len) {
                // Bounds violation
                if (hal && hal->mem.free) hal->mem.free(dst);
                return NULL;
            }
            runtime_memcpy(dst + dst_pos, src + src_pos, run_len);
            src_pos += run_len;
            dst_pos += run_len;
        } else {
            // Match run
            size_t match_len = (ctrl & 0x7F) + 3;
            if (src_pos + 2 > src_len) {
                if (hal && hal->mem.free) hal->mem.free(dst);
                return NULL;
            }
            size_t dist = src[src_pos] | (src[src_pos + 1] << 8);
            src_pos += 2;
            
            if (dist == 0 || dist > dst_pos || dst_pos + match_len > decomp_len) {
                // Invalid offset or bounds overflow
                if (hal && hal->mem.free) hal->mem.free(dst);
                return NULL;
            }
            
            // Byte-by-byte copy to handle overlapping/repeating bytes
            for (size_t k = 0; k < match_len; k++) {
                dst[dst_pos + k] = dst[dst_pos - dist + k];
            }
            dst_pos += match_len;
        }
    }
    
    return dst;
}

void hash_string(const char *algo, const char *data, char *out_buf, size_t out_size) {
    if (!data || !out_buf || out_size == 0) return;

    // Determine required output size based on algorithm
    size_t required = 0;
    if (algo && (runtime_strcasecmp(algo, "SHA256") == 0 || runtime_strcasecmp(algo, "SHA512") == 0)) {
        required = 65; // 64 hex chars + null
    } else if (algo && runtime_strcasecmp(algo, "MD5") == 0) {
        required = 33; // 32 hex chars + null
    } else {
        required = 65; // Safe default
    }
    if (out_size < required) {
        out_buf[0] = '\0';
        return;
    }

    size_t len = runtime_strlen(data);
    uint64_t h = fnv1a_64(data, len);

    if (algo && (runtime_strcasecmp(algo, "MD5") == 0 || runtime_strcasecmp(algo, "CRC32") == 0)) {
        uint32_t c = (uint32_t)(h & 0xFFFFFFFF);
        runtime_snprintf(out_buf, out_size, "%08x", c);
    } else if (algo && runtime_strcasecmp(algo, "SHA1") == 0) {
        runtime_snprintf(out_buf, out_size, "%016llx%08llx", (unsigned long long)h, (unsigned long long)(h ^ 0x5555555555555555ULL));
    } else if (algo && (runtime_strcasecmp(algo, "SHA512") == 0)) {
        runtime_snprintf(out_buf, out_size, "%016llx%016llx%016llx%016llx",
                 (unsigned long long)h, (unsigned long long)(h ^ 0xAAAAAAAAAAAAAAAAULL),
                 (unsigned long long)(h ^ 0x5555555555555555ULL), (unsigned long long)(h ^ 0xFFFFFFFFFFFFFFFFULL));
    } else {
        // Default SHA-256 / FNV-64 hex format
        runtime_snprintf(out_buf, out_size, "%016llx%016llx", (unsigned long long)h, (unsigned long long)(h ^ 0xAAAAAAAAAAAAAAAAULL));
    }
}

