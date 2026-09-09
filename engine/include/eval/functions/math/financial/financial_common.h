// FILENAME: financial_common.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pv.c, fv.c, pmt.c, ipmt.c, ppmt.c, nper.c, rate.c, npv.c, irr.c)
// NEEDS: libcore (math.h, types.h)
// Provides shared financial mathematics helpers for BASIC++.

#ifndef FINANCIAL_COMMON_H
#define FINANCIAL_COMMON_H

#include "runtime/math.h"
#include "types/types.h"
#include "eval/eval.h"
#include "runtime/math/math.h"

static inline double calc_pmt(double rate, double nper, double pv, double fv, int type) {
    if (nper == 0.0) return 0.0;
    if (rate == 0.0) return -(pv + fv) / nper;
    double f = 1.0 + rate * (type ? 1.0 : 0.0);
    double pvif = runtime_pow(1.0 + rate, nper);
    return (-fv - pv * pvif) / (f * (pvif - 1.0) / rate);
}

#endif // FINANCIAL_COMMON_H
