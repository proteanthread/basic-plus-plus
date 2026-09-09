// FILENAME: place.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (place.h, str_math_common.h)
// Provides runtime implementation for the PLACE$ function in BASIC++.

#include "eval/functions/string/manipulation/place.h"
#include "eval/functions/string/manipulation/str_math_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/conv/float_parse.h"

static const LangDesc g_place_desc = {
    .name = "PLACE$",
    .category = "String Arithmetic",
    .syntax = "PLACE$(str_val, place_flag, decimal_digits)",
    .description = "Formats and scales precision of a decimal numeric string (VAX BASIC / BP2).",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_place_register(void) {
    lang_desc_register(&g_place_desc);
}

BValue func_place_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "PLACE$ expects 2 or 3 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch in PLACE$";
        return res;
    }

    int d = 2;
    if (arg_count == 3) {
        if (args[2].type == VAL_STRING) { err->code = ERR_TYPE_MISMATCH; return res; }
        d = (int)args[2].as.number;
    }
    if (d < 0) d = 0;
    if (d > MAX_PRECISION) d = MAX_PRECISION;

    double v = runtime_atof(str_data(args[0].as.string));
    char buf[128];
    runtime_snprintf(buf, sizeof(buf), "%.*f", d, v);

    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}
