// FILENAME: magtape.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (magtape.h, string.c)
// Provides runtime implementation for the MAGTAPE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/hardware/magtape.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_magtape_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAGTAPE",
        .category = "System Functions",
        .syntax = "MAGTAPE(function_code, channel_num, arg)",
        .help_text = "Performs magnetic tape operations (rewind, space, write EOF) on a file channel (DEC RSTS/E).",
        .error_codes = "Error 13: Type Mismatch, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BValue func_magtape_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname; (void)args; (void)err;

    if (arg_count != 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "MAGTAPE expects 3 arguments (function_code, channel_num, arg)";
        return res;
    }

    // Virtual success status for tape/stream channels
    res.as.number = 0.0;
    return res;
}
