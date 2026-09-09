// FILENAME: pmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, pmt.h)
// Provides runtime implementation for the PMT function in BASIC++.

#include "eval/functions/math/financial/pmt.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_pmt_desc = {
    .name = "PMT",
    .category = "Financial",
    .syntax = "x = PMT(rate, nper, pv [, fv [, type]])",
    .description = "Calculates the periodic payment for an annuity based on constant payments and interest rate.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_pmt_register(void) {
    lang_desc_register(&g_pmt_desc);
}

BValue func_pmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "PMT requires at least (rate, nper, pv)";
        return res;
    }
    double rate = args[0].as.number;
    double nper = args[1].as.number;
    double pv   = args[2].as.number;
    double fv   = (arg_count > 3) ? args[3].as.number : 0.0;
    int type    = (arg_count > 4) ? (int)args[4].as.number : 0;

    res.as.number = calc_pmt(rate, nper, pv, fv, type);
    return res;
}
