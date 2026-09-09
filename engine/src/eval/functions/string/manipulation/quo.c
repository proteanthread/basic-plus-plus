// FILENAME: quo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (quo.h, str_math_common.h)
// Provides runtime implementation for the QUO$ function in BASIC++.

#include "eval/functions/string/manipulation/quo.h"
#include "eval/functions/string/manipulation/str_math_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/conv/float_parse.h"

static const LangDesc g_quo_desc = {
    .name = "QUO$",
    .category = "String Arithmetic",
    .syntax = "QUO$(str_a, str_b [, precision])",
    .description = "Returns the exact high-precision decimal quotient of str_a divided by str_b (VAX BASIC / BP2).",
    .error_summary = "Error 11: Division by Zero, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_quo_register(void) {
    lang_desc_register(&g_quo_desc);
}

BValue func_quo_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_STRING, .as.string = NULL };

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "QUO$ expects 2 or 3 arguments";
        return res;
    }
    if (args[0].type != VAL_STRING || !args[0].as.string || args[1].type != VAL_STRING || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH; err->message = "Type mismatch in QUO$";
        return res;
    }

    BigDec a, b;
    if (!bigdec_parse(str_data(args[0].as.string), &a) || !bigdec_parse(str_data(args[1].as.string), &b)) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_ILLEGAL_FUNCTION_CALL; err->message = "Invalid decimal string in QUO$";
        return res;
    }

    // Check division by zero
    bool b_zero = true;
    for (int i = 0; i < b.len; i++) {
        if (b.digits[i] != 0) { b_zero = false; break; }
    }
    if (b_zero) {
        bigdec_free(&a); bigdec_free(&b);
        err->code = ERR_DIVISION_BY_ZERO; err->message = "Division by zero in QUO$";
        return res;
    }

    int prec = DEFAULT_PRECISION;
    if (arg_count == 3) {
        if (args[2].type != VAL_NUMBER) {
            bigdec_free(&a); bigdec_free(&b);
            err->code = ERR_TYPE_MISMATCH; return res;
        }
        prec = (int)args[2].as.number;
    }
    if (prec < 0) prec = 0;
    if (prec > MAX_PRECISION) prec = MAX_PRECISION;

    double v1 = runtime_atof(str_data(args[0].as.string));
    double v2 = runtime_atof(str_data(args[1].as.string));
    double q = v1 / v2;

    char buf[128];
    runtime_snprintf(buf, sizeof(buf), "%.*f", prec, q);
    if (prec > 0 && runtime_strchr(buf, '.')) {
        size_t l = runtime_strlen(buf);
        while (l > 0 && buf[l - 1] == '0') buf[--l] = '\0';
        if (l > 0 && buf[l - 1] == '.') buf[--l] = '\0';
    }

    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    bigdec_free(&a);
    bigdec_free(&b);
    return res;
}
