// FILENAME: rate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, rate.h)
// Provides runtime implementation for the RATE function in BASIC++.

#include "eval/functions/math/financial/rate.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_rate_desc = {
    .name = "RATE",
    .category = "Financial",
    .syntax = "x = RATE(nper, pmt, pv [, fv [, type [, guess]]])",
    .description = "Calculates the interest rate per period of an annuity using Newton-Raphson solver.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_rate_register(void) {
    lang_desc_register(&g_rate_desc);
}

BValue func_rate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "RATE requires at least (nper, pmt, pv)";
        return res;
    }
    double nper = args[0].as.number;
    double pmt  = args[1].as.number;
    double pv   = args[2].as.number;
    double fv   = (arg_count > 3) ? args[3].as.number : 0.0;
    int type    = (arg_count > 4) ? (int)args[4].as.number : 0;
    double rate = (arg_count > 5) ? args[5].as.number : 0.1;

    // Newton-Raphson Solver
    for (int iter = 0; iter < 100; iter++) {
        double f = 1.0 + rate * (type ? 1.0 : 0.0);
        double pvif = runtime_pow(1.0 + rate, nper);
        double y = pv * pvif + pmt * f * (pvif - 1.0) / rate + fv;
        if (runtime_fabs(y) < 1e-10) {
            res.as.number = rate;
            return res;
        }
        // Derivative dy/drate
        double df_drate = (type ? 1.0 : 0.0);
        double dpvif = nper * runtime_pow(1.0 + rate, nper - 1.0);
        double term = (f * (pvif - 1.0)) / rate;
        double dterm = (df_drate * (pvif - 1.0) + f * dpvif - term) / rate;
        double dy = pv * dpvif + pmt * dterm;
        if (dy == 0.0) break;
        double next_rate = rate - y / dy;
        if (runtime_fabs(next_rate - rate) < 1e-10) {
            res.as.number = next_rate;
            return res;
        }
        rate = next_rate;
    }
    res.as.number = rate;
    return res;
}
