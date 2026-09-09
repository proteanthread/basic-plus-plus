// FILENAME: cvdmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (cvdmbf.h, mbf_common.h)
// Provides runtime implementation for the CVDMBF function in BASIC++.

#include "eval/functions/string/conversion/cvdmbf.h"
#include "eval/functions/string/conversion/mbf_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"

static const LangDesc g_cvdmbf_desc = {
    .name = "CVDMBF",
    .category = "Type Conversion",
    .syntax = "x# = CVDMBF(s$)",
    .description = "Converts 8-byte Microsoft Binary Format (MBF) string to double-precision float.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_cvdmbf_register(void) {
    lang_desc_register(&g_cvdmbf_desc);
}

BValue func_cvdmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "CVDMBF expects string argument";
        return res;
    }
    
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    if (len < 8) {
        err->code = 5; err->message = "CVDMBF requires at least 8 bytes";
        return res;
    }
    
    res.as.number = mbf8_to_double((const uint8_t *)data);
    return res;
}
