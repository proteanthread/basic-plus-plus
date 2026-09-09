// FILENAME: isarray.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libengine (isarray.h, string.h, memory.h)
// Provides runtime evaluation for the ISARRAY function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/types/isarray.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_isarray_desc = {
    .name = "ISARRAY",
    .syntax = "result = ISARRAY(expression)",
    .description = "Returns -1 (True) if expression is an array or array descriptor, else 0 (False).",
    .category = "Introspection"
};

void func_isarray_register(void) {
    lang_desc_register(&g_isarray_desc);
}

BValue func_isarray_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1) {
        if (err) {
            err->code = 13;
            err->message = "ISARRAY expects 1 argument";
        }
        return res;
    }

    if (args[0].type == VAL_ARRAY_REF) {
        res.as.number = -1.0;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        str_release(vm_get_str(vm), args[0].as.string);
    }

    return res;
}
