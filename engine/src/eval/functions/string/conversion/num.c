// FILENAME: num.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (num.h, string.c)
// Provides runtime implementation for the NUM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/num.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/conv/num_parse.h"

static const LangDesc g_num_desc = {
    .name = "NUM$",
    .category = "String Functions",
    .syntax = "NUM$(numeric_val) | NUM1$(numeric_val) | VAL%(str_val)",
    .description = "Converts numbers to formatted strings (NUM$ with leading space, NUM1$ unpadded) or strings to integer (VAL%).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_num_register(void) {
    lang_desc_register(&g_num_desc);
}

BValue func_num_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Expected 1 argument";
        return res;
    }

    if (runtime_strcmp(uname, "NUM$") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        char buf[64];
        double n = args[0].as.number;
        if (n >= 0.0) {
            runtime_snprintf(buf, sizeof(buf), " %g", n);
        } else {
            runtime_snprintf(buf, sizeof(buf), "%g", n);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    if (runtime_strcmp(uname, "NUM1$") == 0) {
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        char buf[64];
        runtime_snprintf(buf, sizeof(buf), "%g", args[0].as.number);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    if (runtime_strcmp(uname, "VAL%") == 0) {
        if (args[0].type != VAL_STRING || !args[0].as.string) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        const char *s = str_data(args[0].as.string);
        res.type = VAL_NUMBER;
        res.as.number = (double)runtime_atoi(s);
        return res;
    }

    return res;
}
