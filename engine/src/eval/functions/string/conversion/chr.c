// FILENAME: chr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (chr.h, string.c, vm.h)
// Provides runtime implementation for the CHR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/chr.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_chr_desc = {
    .name = "CHR$",
    .category = "String Functions",
    .syntax = "CHR$(code)",
    .description = "Returns a 1-character string containing the character corresponding to ASCII code (0-255).",
    .error_summary = "Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_chr_register(void) {
    lang_desc_register(&g_chr_desc);
}

BValue func_chr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "CHR$") != 0 && runtime_strcmp(uname, "CHR") != 0) {
        return res;
    }

    if (arg_count < 1) {
        err->code = 13;
        err->message = "CHR$ expects at least one numeric argument";
        return res;
    }

    // Classic 1-argument fast path for 0..255 ASCII
    if (arg_count == 1) {
        if (args[0].type == VAL_STRING) {
            err->code = 13;
            err->message = "CHR$ expects numeric argument";
            return res;
        }

        int code = (int)args[0].as.number;
        if (code < 0) {
            err->code = 5;
            err->message = "Illegal function call in CHR$";
            return res;
        }

        if (code <= 255) {
            char ch = (char)code;
            res.type = VAL_STRING;
            res.as.string = str_create(vm_get_str(vm), &ch, 1);
            return res;
        }

        // UTF-8 code point conversion for code > 255
        char utf8_buf[8];
        size_t utf8_len = 0;
        if (code <= 0x7FF) {
            utf8_buf[0] = (char)(0xC0 | ((code >> 6) & 0x1F));
            utf8_buf[1] = (char)(0x80 | (code & 0x3F));
            utf8_len = 2;
        } else if (code <= 0xFFFF) {
            utf8_buf[0] = (char)(0xE0 | ((code >> 12) & 0x0F));
            utf8_buf[1] = (char)(0x80 | ((code >> 6) & 0x3F));
            utf8_buf[2] = (char)(0x80 | (code & 0x3F));
            utf8_len = 3;
        } else if (code <= 0x10FFFF) {
            utf8_buf[0] = (char)(0xF0 | ((code >> 18) & 0x07));
            utf8_buf[1] = (char)(0x80 | ((code >> 12) & 0x3F));
            utf8_buf[2] = (char)(0x80 | ((code >> 6) & 0x3F));
            utf8_buf[3] = (char)(0x80 | (code & 0x3F));
            utf8_len = 4;
        } else {
            err->code = 5;
            err->message = "Illegal unicode code point in CHR$";
            return res;
        }
        utf8_buf[utf8_len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), utf8_buf, utf8_len);
        return res;
    }

    // Variadic multi-argument CHR$(c1, c2, ...)
    char *out_buf = (char *)(hal_get() ? hal_get()->mem.alloc((size_t)(arg_count * 4 + 1)) : NULL);
    if (!out_buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    size_t pos = 0;
    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            if (args[i].as.string) str_release(vm_get_str(vm), args[i].as.string);
            if (hal_get()) hal_get()->mem.free(out_buf);
            err->code = 13;
            err->message = "CHR$ expects numeric argument";
            return res;
        }

        int code = (int)args[i].as.number;
        if (code < 0) {
            if (hal_get()) hal_get()->mem.free(out_buf);
            err->code = 5;
            err->message = "Illegal function call in CHR$";
            return res;
        }

        if (code <= 255) {
            out_buf[pos++] = (char)code;
        } else if (code <= 0x7FF) {
            out_buf[pos++] = (char)(0xC0 | ((code >> 6) & 0x1F));
            out_buf[pos++] = (char)(0x80 | (code & 0x3F));
        } else if (code <= 0xFFFF) {
            out_buf[pos++] = (char)(0xE0 | ((code >> 12) & 0x0F));
            out_buf[pos++] = (char)(0x80 | ((code >> 6) & 0x3F));
            out_buf[pos++] = (char)(0x80 | (code & 0x3F));
        } else if (code <= 0x10FFFF) {
            out_buf[pos++] = (char)(0xF0 | ((code >> 18) & 0x07));
            out_buf[pos++] = (char)(0x80 | ((code >> 12) & 0x3F));
            out_buf[pos++] = (char)(0x80 | ((code >> 6) & 0x3F));
            out_buf[pos++] = (char)(0x80 | (code & 0x3F));
        }
    }
    out_buf[pos] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, pos);
    if (hal_get()) hal_get()->mem.free(out_buf);
    return res;
}
