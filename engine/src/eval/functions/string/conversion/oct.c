// FILENAME: oct.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (oct.h, string.c)
// Provides runtime implementation for the OCT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/oct.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_oct_register(void) {
    MicroLibMetadata meta = {
        .name = "OCT$",
        .category = "String Functions",
        .syntax = "OCT$(x)",
        .help_text = "Returns the octal string representation of integer x.",
        .error_codes = "Error 13: Type Mismatch (OCT$ expects one numeric argument)"
    };
    microlib_register(&meta);
}
BValue func_oct_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "OCT$ expects one numeric argument";
        return res;
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char tmp[24];
    runtime_snprintf(tmp, sizeof(tmp), "%lo", uv);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), tmp, runtime_strlen(tmp));
    return res;
}
