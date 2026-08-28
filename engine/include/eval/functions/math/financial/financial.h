// FILENAME: financial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, financial.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the FINANCIAL built-in function in BASIC++.
//
// ---- Includes ----

#ifndef FINANCIAL_H
#define FINANCIAL_H

#include "types/types.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BValue func_pv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_fv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_pmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ipmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_ppmt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_nper_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_rate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_npv_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_irr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

void func_financial_register(void);

#ifdef __cplusplus
}
#endif

#endif // FINANCIAL_H
