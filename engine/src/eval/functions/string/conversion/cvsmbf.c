// FILENAME: cvsmbf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (cvsmbf.h, mbf_common.h)
// Provides runtime implementation for the CVSMBF function in BASIC++.

#include "eval/functions/string/conversion/cvsmbf.h"
#include "eval/functions/string/conversion/mbf_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"

static const LangDesc g_cvsmbf_desc = {
    .name = "CVSMBF",
    .category = "Type Conversion",
    .syntax = "x! = CVSMBF(s$)",
    .description = "Converts 4-byte Microsoft Binary Format (MBF) string to single-precision float.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_cvsmbf_register(void) {
    lang_desc_register(&g_cvsmbf_desc);
}

BValue func_cvsmbf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "CVSMBF expects string argument";
        return res;
    }
    
    const char *data = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);
    if (len < 4) {
        err->code = 5; err->message = "CVSMBF requires at least 4 bytes";
        return res;
    }
    
    res.as.number = mbf4_to_double((const uint8_t *)data);
    return res;
}
