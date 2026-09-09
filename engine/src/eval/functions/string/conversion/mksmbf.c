// FILENAME: mksmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (mbf_common.h, mksmbf.h)
// Provides runtime implementation for the MKSMBF$ function in BASIC++.

#include "eval/functions/string/conversion/mksmbf.h"
#include "eval/functions/string/conversion/mbf_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"

static const LangDesc g_mksmbf_desc = {
    .name = "MKSMBF$",
    .category = "Type Conversion",
    .syntax = "s$ = MKSMBF$(x!)",
    .description = "Converts single-precision float to 4-byte Microsoft Binary Format (MBF) string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_mksmbf_register(void) {
    lang_desc_register(&g_mksmbf_desc);
}

BValue func_mksmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13; err->message = "MKSMBF$ expects numeric argument";
        return res;
    }
    
    double val = args[0].as.number;
    uint8_t buf[4];
    double_to_mbf4(val, buf);
    res.as.string = str_create(vm_get_str(vm), (const char *)buf, 4);
    return res;
}
