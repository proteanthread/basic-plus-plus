// FILENAME: cvt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (cvt.h, string.c)
// Provides runtime implementation for the CVT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/cvt.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include <stdbool.h>

void func_cvt_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CVT$%",
        .category = "String Functions",
        .syntax = "CVT$%(str_val) | CVT%$(int_val) | CVT$F(str_val) | CVTF$(flt_val) | CVT$$(str_val, flags) | SWAP%(int_val)",
        .help_text = "Performs DEC PDP-11 / BASIC-PLUS binary word/byte conversion, float packing, character transformations, and byte swapping.",
        .error_codes = "Error 5: Illegal Function Call, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

// @brief Implement DEC CVT$$ string transformation bitmask engine.
//
// Bit 1   (1): Discard parity (clear bit 7).
// Bit 2   (2): Discard all spaces and tabs.
// Bit 4   (4): Discard control characters (CR, LF, FF, ESC, RUBOUT, NULL).
// Bit 8   (8): Discard leading spaces and tabs.
// Bit 16 (16): Compress multiple spaces and tabs to a single space.
// Bit 32 (32): Convert lowercase to uppercase.
// Bit 64 (64): Convert '[' and ']' to '(' and ')'.
// Bit 128 (128): Convert control characters (< 32) to printable equivalents.
// Bit 256 (256): Discard trailing spaces and tabs.
static BValue cvt_string_transform(VMContext *vm, const char *src, size_t src_len, int flags, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)err;

    if (!src || src_len == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(src_len * 2 + 1) : NULL);
    if (!buf) {
        err->code = ERR_OUT_OF_STRING_SPACE;
        err->message = "Out of memory in CVT$$";
        return res;
    }

    size_t out_len = 0;
    bool in_whitespace = false;
    bool seen_non_ws = false;

    for (size_t i = 0; i < src_len; i++) {
        unsigned char c = (unsigned char)src[i];

        // Bit 1: Discard parity (mask to 7 bits)
        if (flags & 1) {
            c &= 0x7F;
        }

        // Bit 4: Discard specific control chars
        if (flags & 4) {
            if (c == 13 || c == 10 || c == 12 || c == 27 || c == 127 || c == 0) {
                continue;
            }
        }

        // Bit 2: Discard all spaces and tabs
        if (flags & 2) {
            if (c == ' ' || c == '\t') {
                continue;
            }
        }

        // Bit 8: Discard leading spaces and tabs
        if (flags & 8) {
            if (!seen_non_ws && (c == ' ' || c == '\t')) {
                continue;
            }
        }

        // Bit 16: Compress multiple whitespace
        if (flags & 16) {
            if (c == ' ' || c == '\t') {
                if (in_whitespace) {
                    continue;
                }
                c = ' ';
                in_whitespace = true;
            } else {
                in_whitespace = false;
            }
        }

        // Bit 32: Convert lowercase to uppercase
        if (flags & 32) {
            if (c >= 'a' && c <= 'z') {
                c = (unsigned char)(c - 32);
            }
        }

        // Bit 64: Convert [ ] to ( )
        if (flags & 64) {
            if (c == '[') c = '(';
            else if (c == ']') c = ')';
        }

        // Bit 128: Convert control characters to printable
        if (flags & 128) {
            if (c < 32 && c != ' ' && c != '\t') {
                c = '?';
            }
        }

        if (c != ' ' && c != '\t') {
            seen_non_ws = true;
        }

        buf[out_len++] = (char)c;
    }

    // Bit 256: Discard trailing spaces and tabs
    if (flags & 256) {
        while (out_len > 0 && (buf[out_len - 1] == ' ' || buf[out_len - 1] == '\t')) {
            out_len--;
        }
    }

    buf[out_len] = '\0';
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, out_len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}

BValue func_cvt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    // CVT$$ or EDIT$: DEC string bitmask transform
    if (runtime_strcmp(uname, "CVT$$") == 0 || runtime_strcmp(uname, "EDIT$") == 0 || runtime_strcmp(uname, "EDIT") == 0) {
        if (arg_count < 1 || arg_count > 2) {
            err->code = ERR_ILLEGAL_FUNCTION_CALL;
            err->message = "CVT$$ / EDIT$ expects 1 or 2 arguments";
            return res;
        }
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "CVT$$ / EDIT$ expects string for first argument";
            return res;
        }
        int flags = 0;
        if (arg_count == 2) {
            if (args[1].type == VAL_STRING) {
                err->code = ERR_TYPE_MISMATCH;
                err->message = "CVT$$ / EDIT$ flags must be numeric";
                return res;
            }
            flags = (int)args[1].as.number;
        }
        const char *s = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);
        return cvt_string_transform(vm, s, len, flags, err);
    }

    if (arg_count != 1) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "CVT / SWAP functions expect 1 argument";
        return res;
    }

    // CVT$% or CVI: String -> 16-bit integer
    if (runtime_strcmp(uname, "CVT$%") == 0 || runtime_strcmp(uname, "CVI") == 0) {
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "CVI / CVT$% expects string argument";
            return res;
        }
        const char *s = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);
        int16_t val = 0;
        if (len >= 1) val |= (uint8_t)s[0];
        if (len >= 2) val |= ((uint8_t)s[1] << 8);
        res.type = VAL_NUMBER;
        res.as.number = (double)val;
        return res;
    }

    // CVT%$ or MKI$: 16-bit integer -> 2-byte binary string
    if (runtime_strcmp(uname, "CVT%$") == 0 || runtime_strcmp(uname, "MKI$") == 0 || runtime_strcmp(uname, "MKI") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "MKI$ / CVT%$ expects numeric argument";
            return res;
        }
        int16_t val = (int16_t)args[0].as.number;
        char buf[3];
        buf[0] = (char)(val & 0xFF);
        buf[1] = (char)((val >> 8) & 0xFF);
        buf[2] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 2);
        return res;
    }

    // CVT$F or CVS: 4-byte binary string -> float
    if (runtime_strcmp(uname, "CVT$F") == 0 || runtime_strcmp(uname, "CVS") == 0) {
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "CVS / CVT$F expects string argument";
            return res;
        }
        const char *s = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);
        float f = 0.0f;
        runtime_memcpy(&f, s, (len >= 4) ? 4 : len);
        res.type = VAL_NUMBER;
        res.as.number = (double)f;
        return res;
    }

    // CVTF$ or MKS$: Float -> 4-byte binary string
    if (runtime_strcmp(uname, "CVTF$") == 0 || runtime_strcmp(uname, "MKS$") == 0 || runtime_strcmp(uname, "MKS") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "MKS$ / CVTF$ expects numeric argument";
            return res;
        }
        float f = (float)args[0].as.number;
        char buf[5];
        runtime_memcpy(buf, &f, 4);
        buf[4] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 4);
        return res;
    }

    // CVD: 8-byte binary string -> double
    if (runtime_strcmp(uname, "CVD") == 0) {
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "CVD expects string argument";
            return res;
        }
        const char *s = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);
        double d = 0.0;
        runtime_memcpy(&d, s, (len >= 8) ? 8 : len);
        res.type = VAL_NUMBER;
        res.as.number = d;
        return res;
    }

    // MKD$: Double -> 8-byte binary string
    if (runtime_strcmp(uname, "MKD$") == 0 || runtime_strcmp(uname, "MKD") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "MKD$ expects numeric argument";
            return res;
        }
        double d = args[0].as.number;
        char buf[9];
        runtime_memcpy(buf, &d, 8);
        buf[8] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 8);
        return res;
    }

    // CVL: 4-byte binary string -> 32-bit long
    if (runtime_strcmp(uname, "CVL") == 0) {
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "CVL expects string argument";
            return res;
        }
        const char *s = str_data(args[0].as.string);
        size_t len = str_len(args[0].as.string);
        int32_t val = 0;
        runtime_memcpy(&val, s, (len >= 4) ? 4 : len);
        res.type = VAL_NUMBER;
        res.as.number = (double)val;
        return res;
    }

    // MKL$: Long -> 4-byte binary string
    if (runtime_strcmp(uname, "MKL$") == 0 || runtime_strcmp(uname, "MKL") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "MKL$ expects numeric argument";
            return res;
        }
        int32_t val = (int32_t)args[0].as.number;
        char buf[5];
        runtime_memcpy(buf, &val, 4);
        buf[4] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, 4);
        return res;
    }

    // SWAP%: 16-bit word byte swap
    if (runtime_strcmp(uname, "SWAP%") == 0 || runtime_strcmp(uname, "SWAP") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            err->message = "SWAP% expects numeric argument";
            return res;
        }
        uint16_t u = (uint16_t)args[0].as.number;
        uint16_t swapped = (uint16_t)(((u & 0xFF) << 8) | ((u >> 8) & 0xFF));
        res.type = VAL_NUMBER;
        res.as.number = (double)(int16_t)swapped;
        return res;
    }

    return res;
}
