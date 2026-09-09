// FILENAME: func_crypto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (crypto_engine.h, crypto_engine.c, memops.h, memops.c)
// NEEDS: libcore (string.h, strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (func_crypto.h)
// Implements CRYPTO.* built-in functions: CRYPTO.ENCRYPT$, CRYPTO.DECRYPT$, CRYPTO.HASH$, CRYPTO.HMAC$, CRYPTO.KEY$.
//
// ---- Includes ----

#include "eval/functions/system/security/func_crypto.h"
#include "runtime/crypto_engine.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_crypto_encrypt_desc = {
    .name = "CRYPTO.ENCRYPT$", .category = "Security & Encryption", .syntax = "CRYPTO.ENCRYPT$(key$, plaintext$)",
    .description = "Encrypts plaintext using symmetric key and returns hex-encoded ciphertext.",
    .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION
};
static const LangDesc g_crypto_decrypt_desc = {
    .name = "CRYPTO.DECRYPT$", .category = "Security & Encryption", .syntax = "CRYPTO.DECRYPT$(key$, ciphertext_hex$)",
    .description = "Decrypts hex-encoded ciphertext using symmetric key and returns plaintext.",
    .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION
};
static const LangDesc g_crypto_hash_desc = {
    .name = "CRYPTO.HASH$", .category = "Security & Encryption", .syntax = "CRYPTO.HASH$(data$)",
    .description = "Computes SHA-256 cryptographic hash of data and returns 64-character hex string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION
};
static const LangDesc g_crypto_hmac_desc = {
    .name = "CRYPTO.HMAC$", .category = "Security & Encryption", .syntax = "CRYPTO.HMAC$(key$, data$)",
    .description = "Computes HMAC-SHA256 message authentication code and returns 64-character hex string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION
};
static const LangDesc g_crypto_key_desc = {
    .name = "CRYPTO.KEY$", .category = "Security & Encryption", .syntax = "CRYPTO.KEY$([bits%])",
    .description = "Generates a cryptographically random symmetric key of specified bit length as hex string.",
    .error_summary = "None", .subsystem = SUBSYSTEM_SERVER, .safety = SAFETY_SAFE, .type = FEATURE_FUNCTION
};

void func_crypto_register(void) {
    lang_desc_register(&g_crypto_encrypt_desc);
    lang_desc_register(&g_crypto_decrypt_desc);
    lang_desc_register(&g_crypto_hash_desc);
    lang_desc_register(&g_crypto_hmac_desc);
    lang_desc_register(&g_crypto_key_desc);
}

BValue func_crypto_encrypt(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (argc < 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    const char *key = str_data(argv[0].as.string);
    const char *plain = str_data(argv[1].as.string);
    size_t plen = str_len(argv[1].as.string);

    char hex_out[1024];
    crypto_encrypt_sim(key, plain, plen, hex_out, sizeof(hex_out));
    res.as.string = str_create(vm_get_str(vm), hex_out, runtime_strlen(hex_out));
    return res;
}

BValue func_crypto_decrypt(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (argc < 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    const char *key = str_data(argv[0].as.string);
    const char *cipher_hex = str_data(argv[1].as.string);
    size_t clen = str_len(argv[1].as.string);

    char plain_out[1024];
    crypto_decrypt_sim(key, cipher_hex, clen, plain_out, sizeof(plain_out));
    res.as.string = str_create(vm_get_str(vm), plain_out, runtime_strlen(plain_out));
    return res;
}

BValue func_crypto_hash(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (argc < 1 || argv[0].type != VAL_STRING) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    const char *data = str_data(argv[0].as.string);
    size_t dlen = str_len(argv[0].as.string);

    char hex_out[65];
    crypto_sha256_hex(data, dlen, hex_out);
    res.as.string = str_create(vm_get_str(vm), hex_out, runtime_strlen(hex_out));
    return res;
}

BValue func_crypto_hmac(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (argc < 2 || argv[0].type != VAL_STRING || argv[1].type != VAL_STRING) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    const char *key = str_data(argv[0].as.string);
    size_t klen = str_len(argv[0].as.string);
    const char *data = str_data(argv[1].as.string);
    size_t dlen = str_len(argv[1].as.string);

    char hex_out[65];
    crypto_hmac_sha256_hex(key, klen, data, dlen, hex_out);
    res.as.string = str_create(vm_get_str(vm), hex_out, runtime_strlen(hex_out));
    return res;
}

BValue func_crypto_key(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    int bits = 256;
    if (argc >= 1 && (argv[0].type == VAL_NUMBER || argv[0].type == VAL_INTEGER)) {
        bits = (int)argv[0].as.number;
    }

    char key_hex[128];
    crypto_keygen(bits, key_hex, sizeof(key_hex));
    res.as.string = str_create(vm_get_str(vm), key_hex, runtime_strlen(key_hex));
    return res;
}
