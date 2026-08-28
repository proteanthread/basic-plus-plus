// FILENAME: tek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, tek.h)
// Provides runtime implementation for the TEK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/format/tek.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_tek_register(void) {
    static const MicroLibMetadata meta = {
        .name = "TEK$/VEC$",
        .category = "Graphics & Vector Telemetry",
        .syntax = "TEK$(x, y) / VEC$(x1, y1, x2, y2)",
        .help_text = "Generates 10-bit Tektronix 4010/4014 vector coordinate sequence (TEK$) and vector line sequence (VEC$).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

static void encode_tek_coord(int x, int y, char out[4]) {
    if (x < 0) x = 0;
    if (x > 1023) x = 1023;
    if (y < 0) y = 0;
    if (y > 1023) y = 1023;

    out[0] = (char)(0x20 | ((y >> 5) & 0x1F)); // Hi-Y
    out[1] = (char)(0x60 | (y & 0x1F));        // Lo-Y
    out[2] = (char)(0x20 | ((x >> 5) & 0x1F)); // Hi-X
    out[3] = (char)(0x40 | (x & 0x1F));        // Lo-X
}

BValue func_tek_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (runtime_strcmp(uname, "TEK") != 0 && runtime_strcmp(uname, "TEK$") != 0 &&
        runtime_strcmp(uname, "_TEK") != 0 && runtime_strcmp(uname, "_TEK$") != 0 &&
        runtime_strcmp(uname, "GFX.TEK") != 0 && runtime_strcmp(uname, "GFX.TEK$") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "TEK$ expects 2 numeric arguments (x, y)";
        return res;
    }

    int x = (int)args[0].as.number;
    int y = (int)args[1].as.number;
    char buf[5];
    encode_tek_coord(x, y, buf);
    buf[4] = '\0';

    res.as.string = str_create(vm_get_str(vm), buf, 4);
    return res;
}

BValue func_vec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (runtime_strcmp(uname, "VEC") != 0 && runtime_strcmp(uname, "VEC$") != 0 &&
        runtime_strcmp(uname, "_VEC") != 0 && runtime_strcmp(uname, "_VEC$") != 0 &&
        runtime_strcmp(uname, "GFX.VEC") != 0 && runtime_strcmp(uname, "GFX.VEC$") != 0) {
        return res;
    }

    if (arg_count != 4 ||
        args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER ||
        args[2].type != VAL_NUMBER || args[3].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "VEC$ expects 4 numeric arguments (x1, y1, x2, y2)";
        return res;
    }

    int x1 = (int)args[0].as.number;
    int y1 = (int)args[1].as.number;
    int x2 = (int)args[2].as.number;
    int y2 = (int)args[3].as.number;

    char buf[10];
    buf[0] = 0x1D; // GS (Group Separator / Tektronix Enter Vector Mode)
    encode_tek_coord(x1, y1, buf + 1);
    encode_tek_coord(x2, y2, buf + 5);
    buf[9] = '\0';

    res.as.string = str_create(vm_get_str(vm), buf, 9);
    return res;
}
