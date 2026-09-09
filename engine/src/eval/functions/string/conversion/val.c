// FILENAME: val.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, val.h)
// Provides runtime implementation for the VAL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/val.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "runtime/conv/float_parse.h"
#include "runtime/conv/num_parse.h"

static const LangDesc g_val_desc = {
    .name = "VAL",
    .category = "String Functions",
    .syntax = "VAL(str$)",
    .description = "Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number.",
    .error_summary = "Error 13: Type Mismatch (VAL expects one string argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_val_register(void) {
    lang_desc_register(&g_val_desc);
}

BValue func_val_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "VAL") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "VAL expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    res.type = VAL_NUMBER;
    const char *s = str_data(sr);
    if (s) {
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '&' && (s[1] == 'h' || s[1] == 'H')) {
            res.as.number = (double)runtime_strtoull(s + 2, NULL, 16);
        } else if (*s == '&' && (s[1] == 'o' || s[1] == 'O')) {
            res.as.number = (double)runtime_strtoull(s + 2, NULL, 8);
        } else if (*s == '&' && (s[1] == 'b' || s[1] == 'B')) {
            res.as.number = (double)runtime_strtoull(s + 2, NULL, 2);
        } else {
            res.as.number = runtime_strtod(s, NULL);
        }
    }
    str_release(vm_get_str(vm), sr);
    return res;
}
