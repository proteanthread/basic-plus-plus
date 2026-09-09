// FILENAME: month.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, month.h)
// Provides runtime implementation for the MONTH function in BASIC++.

#include "eval/functions/datetime/month.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_month_desc = {
    .name = "MONTH",
    .category = "Date and Time",
    .syntax = "MONTH(serial#)",
    .description = "Returns month of the year (1-12) for specified Microsoft Serial Date.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_month_register(void) {
    lang_desc_register(&g_month_desc);
}

BValue func_month_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) {
        struct tm tm_b;
        struct tm *lt = get_current_local_tm(&tm_b);
        res.as.number = (double)(lt ? lt->tm_mon + 1 : 1);
        return res;
    }
    int m = 1;
    serial_to_ymd(get_serial_arg(args), NULL, &m, NULL, NULL);
    res.as.number = (double)m;
    return res;
}
