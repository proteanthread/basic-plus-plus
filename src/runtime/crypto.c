/**
 * @file crypto.c
 * @brief Cryptographic and hashing extensions.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements HASH$, CRC, and GUID$ functions.
 * - Why it exists: Fulfills Phase 2 extension requirements for modern basic features.
 * - Why it works this way: Uses standard algorithms like FNV-1a (64-bit), CRC-64 ECMA, and UUIDv4 generation via internal RNG.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Internal algorithms can be swapped or expanded.
 * - What cannot be changed: The function signatures required by the registry.
 * - What to expect: Standard outputs (hex strings for hashes/GUIDs, integers for CRC).
 * - What to do if something breaks: Check string references and memory contexts.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17, 8-bit characters.
 * - Portability concerns: srand/rand are used; may need platform-specific secure random generation later.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add new hash types like SHA-256 by parsing an optional second argument.
 */

#include "bpp_funcreg.h"
#include "bpp_vm.h"
#include "bpp_strings.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* FNV-1a 64-bit */
static uint64_t fnv1a_64(const char *data, size_t len) {
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t)(unsigned char)data[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

/* HASH$(string$) -> Hex String */
BValue crypto_hash_func(BValue *args, int argc, void *rt) {
    VMContext *vm = (VMContext *)rt;
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    uint64_t h = fnv1a_64(data, len);
    char hex[17];
    snprintf(hex, sizeof(hex), "%016llx", (unsigned long long)h);
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), hex, 16);
    return res;
}

/* CRC-64 ECMA polynomial 0xC96C5795D7870F42 */
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

/* CRC(string$) -> Number */
BValue crypto_crc_func(BValue *args, int argc, void *rt) {
    (void)rt; /* Unused */
    if (argc < 1 || args[0].type != VAL_STRING) {
        return bval_float(0);
    }
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    uint64_t c = crc64_ecma(data, len);
    /* In BASIC++, numbers are typically doubles, but we return a double representing the CRC. 
       Note: 64-bit unsigned may lose precision in double (53 bits). But we must use double to remain compatible. */
    return bval_float((double)c);
}

/* GUID$() -> String */
BValue crypto_guid_func(BValue *args, int argc, void *rt) {
    (void)args; (void)argc;
    VMContext *vm = (VMContext *)rt;
    char guid[37];
    uint32_t r1 = (uint32_t)rand();
    uint32_t r2 = (uint32_t)rand();
    uint32_t r3 = (uint32_t)rand();
    uint32_t r4 = (uint32_t)rand();
    
    r2 = (r2 & 0xFFFF0FFF) | 0x00004000; /* Version 4 */
    r3 = (r3 & 0x3FFFFFFF) | 0x80000000; /* Variant 1 */
    
    snprintf(guid, sizeof(guid), "%08x-%04x-%04x-%04x-%04x%08x",
             r1, r2 >> 16, r2 & 0xFFFF, r3 >> 16, r3 & 0xFFFF, r4);
             
    BValue res;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), guid, 36);
    return res;
}

void register_crypto_functions(void) {
    FunctionEntry fe;
    memset(&fe, 0, sizeof(fe));
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
