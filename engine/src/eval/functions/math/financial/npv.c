// FILENAME: npv.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, npv.h)
// Provides runtime implementation for the NPV function in BASIC++.

#include "eval/functions/math/financial/npv.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_npv_desc = {
    .name = "NPV",
    .category = "Financial",
    .syntax = "x = NPV(rate, val1, val2, ...)",
    .description = "Calculates the net present value of an investment based on a discount rate and periodic cash flows.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_npv_register(void) {
    lang_desc_register(&g_npv_desc);
}

BValue func_npv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 2) {
        err->code = 13; err->message = "NPV requires (rate, value1, ...)";
        return res;
    }
    double rate = args[0].as.number;
    double sum = 0.0;
    for (int i = 1; i < arg_count; i++) {
        sum += args[i].as.number / runtime_pow(1.0 + rate, (double)i);
    }
    res.as.number = sum;
    return res;
}
