// FILENAME: chr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (chr.h, string.c, vm.h)
// Provides runtime implementation for the CHR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/chr.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_chr_register(void) {
    MicroLibMetadata meta = {
        .name = "CHR$",
        .category = "String Functions",
        .syntax = "CHR$(code)",
        .help_text = "Returns a 1-character string containing the character corresponding to ASCII code (0-255).",
        .error_codes = "Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_chr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "CHR$") != 0 && runtime_strcmp(uname, "CHR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "CHR$ expects one numeric argument";
        return res;
    }

    int code = (int)args[0].as.number;
    if (code < 0 || code > 255) {
        err->code = 5;
        err->message = "Illegal function call in CHR$";
        return res;
    }

    char ch = (char)code;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), &ch, 1);
    return res;
}
