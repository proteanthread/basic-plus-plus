// FILENAME: oct.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (oct.h, string.c)
// Provides runtime implementation for the OCT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/oct.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_oct_desc = {
    .name = "OCT$",
    .category = "String Functions",
    .syntax = "OCT$(x)",
    .description = "Returns the octal string representation of integer x.",
    .error_summary = "Error 13: Type Mismatch (OCT$ expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_oct_register(void) {
    lang_desc_register(&g_oct_desc);
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
