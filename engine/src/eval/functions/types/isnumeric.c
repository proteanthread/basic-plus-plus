// FILENAME: isnumeric.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libengine (isnumeric.h, string.h, memory.h)
// Provides runtime evaluation for the ISNUMERIC function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/types/isnumeric.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/math/basic.h"
#include "runtime/conv/float_parse.h"

static const LangDesc g_isnumeric_desc = {
    .name = "ISNUMERIC",
    .syntax = "result = ISNUMERIC(expression)",
    .description = "Returns -1 (True) if expression evaluates to or parses as a valid numeric value, else 0 (False).",
    .category = "Introspection"
};

void func_isnumeric_register(void) {
    lang_desc_register(&g_isnumeric_desc);
}

BValue func_isnumeric_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count != 1) {
        if (err) {
            err->code = 13;
            err->message = "ISNUMERIC expects 1 argument";
        }
        return res;
    }

    if (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) {
        res.as.number = -1.0;
        return res;
    }

    if (args[0].type == VAL_STRING) {
        const char *s = args[0].as.string ? str_data(args[0].as.string) : "";
        while (*s && runtime_isspace((unsigned char)*s)) s++;
        if (*s == '\0') {
            if (args[0].as.string) str_release(vm_get_str(vm), args[0].as.string);
            return res;
        }

        char *endptr = NULL;
        (void)runtime_strtod(s, &endptr);
        if (endptr && endptr != s) {
            while (*endptr && runtime_isspace((unsigned char)*endptr)) endptr++;
            if (*endptr == '\0') {
                res.as.number = -1.0;
            }
        }
        if (args[0].as.string) str_release(vm_get_str(vm), args[0].as.string);
        return res;
    }

    return res;
}
