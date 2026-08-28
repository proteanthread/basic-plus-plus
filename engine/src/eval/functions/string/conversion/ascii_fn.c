// FILENAME: ascii_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ascii_fn.h, string.c)
// Provides runtime implementation for the ASCII_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/ascii_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_ascii_fn_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ASCII",
        .category = "String Functions",
        .syntax = "ASCII(str_expr) | NUM(char_expr)",
        .help_text = "Returns the ASCII numeric value of the first character of a string (DEC PDP-11 / HP 2000).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BValue func_ascii_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    if (arg_count != 1) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Expected 1 string argument";
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "Type mismatch: expected string argument";
        return res;
    }

    size_t len = str_len(args[0].as.string);
    if (len == 0) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Illegal function call: empty string";
        return res;
    }

    const char *data = str_data(args[0].as.string);
    res.as.number = (double)((unsigned char)data[0]);
    return res;
}
