// FILENAME: financial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (financial.h, math.c, string.c)
// Provides runtime implementation for the FINANCIAL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/financial/financial.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"

#include "runtime/string.h"
// Helper to compute PMT
static double calc_pmt(double rate, double nper, double pv, double fv, int type) {
    if (nper == 0.0) return 0.0;
    if (rate == 0.0) return -(pv + fv) / nper;
    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double pvif = runtime_pow(1.0 + rate, nper);
    return (-fv - pv * pvif) / (f * (pvif - 1.0) / rate);
}

BValue func_pv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "PV requires at least (rate, nper, pmt)";
        return res;
    }
    double rate = args[0].as.number;
    double nper = args[1].as.number;
    double pmt  = args[2].as.number;
    double fv   = (arg_count > 3) ? args[3].as.number : 0.0;
    int type    = (arg_count > 4) ? (int)args[4].as.number : 0;

    if (rate == 0.0) {
        res.as.number = -(fv + pmt * nper);
        return res;
    }

    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double pvif = runtime_pow(1.0 + rate, nper);
    res.as.number = (-fv - pmt * f * (pvif - 1.0) / rate) / pvif;
    return res;
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

BValue func_ppmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm; (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 4) {
        err->code = 13; err->message = "PPMT requires at least (rate, per, nper, pv)";
        return res;
    }
    BValue pmt_val = func_pmt_eval(vm, uname, (arg_count >= 5 ? 5 : 4), (BValue[]){ args[0], args[2], args[3], (arg_count > 4 ? args[4] : (BValue){.type=VAL_NUMBER, .as.number=0.0}), (arg_count > 5 ? args[5] : (BValue){.type=VAL_NUMBER, .as.number=0.0}) }, err);
    if (err->code != 0) return res;

    BValue ipmt_val = func_ipmt_eval(vm, uname, arg_count, args, err);
    if (err->code != 0) return res;

    res.as.number = pmt_val.as.number - ipmt_val.as.number;
    return res;
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

void func_financial_register(void) {
    static const MicroLibMetadata meta_pv = { .name = "PV", .category = "Financial", .syntax = "x = PV(rate, nper, pmt [, fv [, type]])" };
    static const MicroLibMetadata meta_fv = { .name = "FV", .category = "Financial", .syntax = "x = FV(rate, nper, pmt [, pv [, type]])" };
    static const MicroLibMetadata meta_pmt = { .name = "PMT", .category = "Financial", .syntax = "x = PMT(rate, nper, pv [, fv [, type]])" };
    static const MicroLibMetadata meta_ipmt = { .name = "IPMT", .category = "Financial", .syntax = "x = IPMT(rate, per, nper, pv [, fv [, type]])" };
    static const MicroLibMetadata meta_ppmt = { .name = "PPMT", .category = "Financial", .syntax = "x = PPMT(rate, per, nper, pv [, fv [, type]])" };
    static const MicroLibMetadata meta_nper = { .name = "NPER", .category = "Financial", .syntax = "x = NPER(rate, pmt, pv [, fv [, type]])" };
    static const MicroLibMetadata meta_rate = { .name = "RATE", .category = "Financial", .syntax = "x = RATE(nper, pmt, pv [, fv [, type [, guess]]])" };
    static const MicroLibMetadata meta_npv = { .name = "NPV", .category = "Financial", .syntax = "x = NPV(rate, val1, val2, ...)" };
    static const MicroLibMetadata meta_irr = { .name = "IRR", .category = "Financial", .syntax = "x = IRR(val1, val2, ...)" };
    microlib_register(&meta_pv);
    microlib_register(&meta_fv);
    microlib_register(&meta_pmt);
    microlib_register(&meta_ipmt);
    microlib_register(&meta_ppmt);
    microlib_register(&meta_nper);
    microlib_register(&meta_rate);
    microlib_register(&meta_npv);
    microlib_register(&meta_irr);
}

