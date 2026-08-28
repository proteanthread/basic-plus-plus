// FILENAME: len.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (len.h, string.c, vm.h)
// Provides runtime implementation for the LEN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/format/len.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_len_register(void) {
    MicroLibMetadata meta = {
        .name = "LEN",
        .category = "String Functions",
        .syntax = "LEN(str$)",
        .help_text = "Returns the number of characters in str$.",
        .error_codes = "Error 13: Type Mismatch (LEN expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_len_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LEN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "LEN expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    res.type = VAL_NUMBER;
    res.as.number = (double)str_len(sr);
    str_release(vm_get_str(vm), sr);
    return res;
}
