// FILENAME: fv.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, fv.h)
// Provides runtime implementation for the FV function in BASIC++.

#include "eval/functions/math/financial/fv.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_fv_desc = {
    .name = "FV",
    .category = "Financial",
    .syntax = "x = FV(rate, nper, pmt [, pv [, type]])",
    .description = "Calculates the future value of an investment based on periodic, constant payments.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_fv_register(void) {
    lang_desc_register(&g_fv_desc);
}

BValue func_fv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "FV requires at least (rate, nper, pmt)";
        return res;
    }
    double rate = args[0].as.number;
    double nper = args[1].as.number;
    double pmt  = args[2].as.number;
    double pv   = (arg_count > 3) ? args[3].as.number : 0.0;
    int type    = (arg_count > 4) ? (int)args[4].as.number : 0;

    if (rate == 0.0) {
        res.as.number = -(pv + pmt * nper);
        return res;
    }

    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double pvif = runtime_pow(1.0 + rate, nper);
    res.as.number = -pv * pvif - pmt * f * (pvif - 1.0) / rate;
    return res;
}
