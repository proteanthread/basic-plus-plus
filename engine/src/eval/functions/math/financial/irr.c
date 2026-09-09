// FILENAME: irr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, irr.h)
// Provides runtime implementation for the IRR function in BASIC++.

#include "eval/functions/math/financial/irr.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_irr_desc = {
    .name = "IRR",
    .category = "Financial",
    .syntax = "x = IRR(val1, val2, ...)",
    .description = "Calculates the internal rate of return for a series of periodic cash flows.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_irr_register(void) {
    lang_desc_register(&g_irr_desc);
}

BValue func_irr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 2) {
        err->code = 13; err->message = "IRR requires at least 2 cash flow values";
        return res;
    }
    double rate = 0.1;
    for (int iter = 0; iter < 100; iter++) {
        double npv = 0.0;
        double d_npv = 0.0;
        for (int i = 0; i < arg_count; i++) {
            double denom = runtime_pow(1.0 + rate, (double)i);
            npv += args[i].as.number / denom;
            if (i > 0) {
                d_npv -= (double)i * args[i].as.number / (denom * (1.0 + rate));
            }
        }
        if (runtime_fabs(npv) < 1e-10) {
            res.as.number = rate;
            return res;
        }
        if (d_npv == 0.0) break;
        double next_rate = rate - npv / d_npv;
        if (runtime_fabs(next_rate - rate) < 1e-10) {
            res.as.number = next_rate;
            return res;
        }
        rate = next_rate;
    }
    res.as.number = rate;
    return res;
}
