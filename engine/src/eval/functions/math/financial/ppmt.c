// FILENAME: ppmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, ipmt.h, pmt.h, ppmt.h)
// Provides runtime implementation for the PPMT function in BASIC++.

#include "eval/functions/math/financial/ppmt.h"
#include "eval/functions/math/financial/pmt.h"
#include "eval/functions/math/financial/ipmt.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_ppmt_desc = {
    .name = "PPMT",
    .category = "Financial",
    .syntax = "x = PPMT(rate, per, nper, pv [, fv [, type]])",
    .description = "Calculates the payment on the principal for an investment for a given period.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_ppmt_register(void) {
    lang_desc_register(&g_ppmt_desc);
}

BValue func_ppmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 4) {
        err->code = 13; err->message = "PPMT requires at least (rate, per, nper, pv)";
        return res;
    }
    BValue pmt_args[5];
    pmt_args[0] = args[0]; // rate
    pmt_args[1] = args[2]; // nper
    pmt_args[2] = args[3]; // pv
    pmt_args[3] = (arg_count > 4) ? args[4] : (BValue){.type = VAL_NUMBER, .as.number = 0.0}; // fv
    pmt_args[4] = (arg_count > 5) ? args[5] : (BValue){.type = VAL_NUMBER, .as.number = 0.0}; // type

    BValue pmt_val = func_pmt_eval(vm, uname, (arg_count >= 5 ? 5 : 4), pmt_args, err);
    if (err->code != 0) return res;

    BValue ipmt_val = func_ipmt_eval(vm, uname, arg_count, args, err);
    if (err->code != 0) return res;

    res.as.number = pmt_val.as.number - ipmt_val.as.number;
    return res;
}
