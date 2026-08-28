// FILENAME: mbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (math.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (math.c, mbf.h, string.c)
// Provides runtime implementation for the MBF built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/mbf.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/math.h"
#include <stdint.h>

static double mbf4_to_double(const uint8_t *b) {
    uint8_t exp = b[3];
    if (exp == 0) return 0.0;
    
    double mant = ((double)((b[2] & 0x7F) | 0x80) * 65536.0 + (double)b[1] * 256.0 + (double)b[0]) / 16777216.0;
    double val = runtime_ldexp(mant, (int)exp - 128);
    if (b[2] & 0x80) val = -val;
    return val;
}

static double mbf8_to_double(const uint8_t *b) {
    uint8_t exp = b[7];
    if (exp == 0) return 0.0;
    
    double hi = ((double)((b[6] & 0x7F) | 0x80) * 256.0 + (double)b[5]) * 65536.0 + ((double)b[4] * 256.0 + (double)b[3]);
    double lo = ((double)b[2] * 256.0 + (double)b[1]) * 256.0 + (double)b[0];
    double mant = (hi * 16777216.0 + lo) / 72057594037927936.0;
    
    double val = runtime_ldexp(mant, (int)exp - 128);
    if (b[6] & 0x80) val = -val;
    return val;
}

static void double_to_mbf4(double val, uint8_t *b) {
    runtime_memset(b, 0, 4);
    if (val == 0.0) return;
    
    int exp = 0;
    double mant = runtime_frexp(runtime_fabs(val), &exp);
    int mbf_exp = exp + 128;
    if (mbf_exp <= 0) return; // Underflow
    if (mbf_exp > 255) mbf_exp = 255; // Overflow clamp
    
    uint32_t m_int = (uint32_t)(mant * 16777216.0);
    b[3] = (uint8_t)mbf_exp;
    b[2] = (uint8_t)((m_int >> 16) & 0x7F) | (val < 0.0 ? 0x80 : 0);
    b[1] = (uint8_t)((m_int >> 8) & 0xFF);
    b[0] = (uint8_t)(m_int & 0xFF);
}

static void double_to_mbf8(double val, uint8_t *b) {
    runtime_memset(b, 0, 8);
    if (val == 0.0) return;
    
    int exp = 0;
    double mant = runtime_frexp(runtime_fabs(val), &exp);
    int mbf_exp = exp + 128;
    if (mbf_exp <= 0) return;
    if (mbf_exp > 255) mbf_exp = 255;
    
    uint64_t m_int = (uint64_t)(mant * 72057594037927936.0);

    b[7] = (uint8_t)mbf_exp;
    b[6] = (uint8_t)((m_int >> 48) & 0x7F) | (val < 0.0 ? 0x80 : 0);
    b[5] = (uint8_t)((m_int >> 40) & 0xFF);
    b[4] = (uint8_t)((m_int >> 32) & 0xFF);
    b[3] = (uint8_t)((m_int >> 24) & 0xFF);
    b[2] = (uint8_t)((m_int >> 16) & 0xFF);
    b[1] = (uint8_t)((m_int >> 8) & 0xFF);
    b[0] = (uint8_t)(m_int & 0xFF);
}

BValue func_cvsmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "CVSMBF expects string argument";
        return res;
    }
    
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    if (len < 4) {
        err->code = 5; err->message = "CVSMBF requires at least 4 bytes";
        return res;
    }
    
    res.as.number = mbf4_to_double((const uint8_t *)data);
    return res;
}

BValue func_cvdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "CVDMBF expects string argument";
        return res;
    }
    
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    if (len < 8) {
        err->code = 5; err->message = "CVDMBF requires at least 8 bytes";
        return res;
    }
    
    res.as.number = mbf8_to_double((const uint8_t *)data);
    return res;
}

BValue func_mksmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13; err->message = "MKSMBF$ expects numeric argument";
        return res;
    }
    
    double val = args[0].as.number;
    uint8_t buf[4];
    double_to_mbf4(val, buf);
    res.as.string = str_create(vm_get_str(vm), (const char *)buf, 4);
    return res;
}

BValue func_mkdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13; err->message = "MKDMBF$ expects numeric argument";
        return res;
    }
    
    double val = args[0].as.number;
    uint8_t buf[8];
    double_to_mbf8(val, buf);
    res.as.string = str_create(vm_get_str(vm), (const char *)buf, 8);
    return res;
}

void func_mbf_register(void) {
    static const MicroLibMetadata meta_cvs = {
        .name = "CVSMBF", .category = "Type Conversion",
        .syntax = "x! = CVSMBF(s$)", .help_text = "Converts 4-byte Microsoft Binary Format (MBF) string to single-precision float."
    };
    static const MicroLibMetadata meta_cvd = {
        .name = "CVDMBF", .category = "Type Conversion",
        .syntax = "x# = CVDMBF(s$)", .help_text = "Converts 8-byte Microsoft Binary Format (MBF) string to double-precision float."
    };
    static const MicroLibMetadata meta_mks = {
        .name = "MKSMBF$", .category = "Type Conversion",
        .syntax = "s$ = MKSMBF$(x!)", .help_text = "Converts single-precision float to 4-byte Microsoft Binary Format (MBF) string."
    };
    static const MicroLibMetadata meta_mkd = {
        .name = "MKDMBF$", .category = "Type Conversion",
        .syntax = "s$ = MKDMBF$(x#)", .help_text = "Converts double-precision float to 8-byte Microsoft Binary Format (MBF) string."
    };
    microlib_register(&meta_cvs);
    microlib_register(&meta_cvd);
    microlib_register(&meta_mks);
    microlib_register(&meta_mkd);
}
