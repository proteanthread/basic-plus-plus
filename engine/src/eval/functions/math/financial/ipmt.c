// FILENAME: ipmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, ipmt.h)
// Provides runtime implementation for the IPMT function in BASIC++.

#include "eval/functions/math/financial/ipmt.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_ipmt_desc = {
    .name = "IPMT",
    .category = "Financial",
    .syntax = "x = IPMT(rate, per, nper, pv [, fv [, type]])",
    .description = "Calculates the interest payment for an investment for a given period.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_ipmt_register(void) {
    lang_desc_register(&g_ipmt_desc);
}

BValue func_ipmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 4) {
        err->code = 13; err->message = "IPMT requires at least (rate, per, nper, pv)";
        return res;
    }
    double rate = args[0].as.number;
    double per  = args[1].as.number;
    double nper = args[2].as.number;
    double pv   = args[3].as.number;
    double fv   = (arg_count > 4) ? args[4].as.number : 0.0;
    int type    = (arg_count > 5) ? (int)args[5].as.number : 0;

    if (per < 1.0 || per > nper) {
        err->code = 5; err->message = "IPMT period out of range";
        return res;
    }

    double pmt = calc_pmt(rate, nper, pv, fv, type);
    if (rate == 0.0) {
        res.as.number = 0.0;
        return res;
    }

    if (per == 1.0 && type == 1) {
        res.as.number = 0.0;
        return res;
    }

    double pvif = runtime_pow(1.0 + rate, per - 1.0);
    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double balance = pv * pvif + pmt * f * (pvif - 1.0) / rate;
    res.as.number = -balance * rate;
    return res;
}
