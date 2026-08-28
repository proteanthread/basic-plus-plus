// FILENAME: rad.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (rad.h, string.c)
// Provides runtime implementation for the RAD built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/rad.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_rad_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RAD$",
        .category = "String Functions",
        .syntax = "RAD$(numeric_val)",
        .help_text = "Converts an integer to a DEC Radix-50 encoded 3-character string.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

static char rad50_char(int code) {
    static const char *rad_table = " ABCDEFGHIJKLMNOPQRSTUVWXYZ$.%0123456789";
    if (code >= 0 && code < 40) {
        return rad_table[code];
    }
    return ' ';
}

BValue func_rad_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "RAD$ expects one numeric argument";
        return res;
    }

    uint32_t val = (uint32_t)args[0].as.number;
    char buf[7] = {0};

    if (val <= 0xFFFF) {
        // 16-bit 3-character RAD50
        buf[2] = rad50_char(val % 40);
        val /= 40;
        buf[1] = rad50_char(val % 40);
        val /= 40;
        buf[0] = rad50_char(val % 40);
        buf[3] = '\0';
    } else {
        // 32-bit 6-character RAD50
        uint16_t w1 = (uint16_t)(val >> 16);
        uint16_t w2 = (uint16_t)(val & 0xFFFF);
        buf[2] = rad50_char(w1 % 40); w1 /= 40;
        buf[1] = rad50_char(w1 % 40); w1 /= 40;
        buf[0] = rad50_char(w1 % 40);

        buf[5] = rad50_char(w2 % 40); w2 /= 40;
        buf[4] = rad50_char(w2 % 40); w2 /= 40;
        buf[3] = rad50_char(w2 % 40);
        buf[6] = '\0';
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}
