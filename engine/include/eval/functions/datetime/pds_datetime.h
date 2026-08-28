// FILENAME: pds_datetime.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c, pds_datetime.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the PDS_DATETIME built-in function in BASIC++.
//
// ---- Includes ----

#ifndef PDS_DATETIME_H
#define PDS_DATETIME_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Serial Date and Time Constructors
BValue func_dateserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_timeserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Date and Time String Value Parsers
BValue func_datevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_timevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Date Decomposition Functions
BValue func_day_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_month_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_year_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_weekday_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Time Decomposition Functions
BValue func_hour_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_minute_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_second_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Dual Epoch Helpers
BValue func_unixtime_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_epochdate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

void func_pds_datetime_register(void);

#ifdef __cplusplus
}
#endif

#endif // PDS_DATETIME_H
