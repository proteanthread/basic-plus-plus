// FILENAME: str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libengine (str.h, string.c, vm.h)
// Provides runtime implementation for the STR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/str.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_str_register(void) {
    MicroLibMetadata meta = {
        .name = "STR$",
        .category = "String Functions",
        .syntax = "STR$(x)",
        .help_text = "Returns the string representation of numeric expression x.",
        .error_codes = "Error 13: Type Mismatch (STR$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "STR$") != 0 && runtime_strcmp(uname, "STR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "STR$ expects one numeric argument";
        return res;
    }

    char buf[64];
    num_format_display(buf, sizeof(buf), args[0].as.number, true, false);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}
