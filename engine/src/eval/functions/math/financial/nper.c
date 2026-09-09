// FILENAME: nper.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (financial_common.h, nper.h)
// Provides runtime implementation for the NPER function in BASIC++.

#include "eval/functions/math/financial/nper.h"
#include "eval/functions/math/financial/financial_common.h"
#include "runtime/language_descriptor.h"
#include "runtime/math/math.h"

static const LangDesc g_nper_desc = {
    .name = "NPER",
    .category = "Financial",
    .syntax = "x = NPER(rate, pmt, pv [, fv [, type]])",
    .description = "Calculates the number of periods for an investment based on periodic, constant payments.",
    .error_summary = "Error 5: Illegal Function Call, Error 11: Division by Zero, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_nper_register(void) {
    lang_desc_register(&g_nper_desc);
}

BValue func_nper_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "NPER requires at least (rate, pmt, pv)";
        return res;
    }
    double rate = args[0].as.number;
    double pmt  = args[1].as.number;
    double pv   = args[2].as.number;
    double fv   = (arg_count > 3) ? args[3].as.number : 0.0;
    int type    = (arg_count > 4) ? (int)args[4].as.number : 0;

    if (rate == 0.0) {
        if (pmt == 0.0) { err->code = 11; err->message = "Division by zero in NPER"; return res; }
        res.as.number = -(pv + fv) / pmt;
        return res;
    }

    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double num = (-fv * rate + pmt * f) / (pv * rate + pmt * f);
    if (num <= 0.0) {
        err->code = 5; err->message = "Invalid arguments in NPER";
        return res;
    }

    res.as.number = runtime_log(num) / runtime_log(1.0 + rate);
    return res;
}
