// FILENAME: timeserial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (timeserial.h, datetime_common.h)
// Provides runtime implementation for the TIMESERIAL function in BASIC++.

#include "eval/functions/datetime/timeserial.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_timeserial_desc = {
    .name = "TIMESERIAL",
    .category = "Date and Time",
    .syntax = "TIMESERIAL(hour%, minute%, second%)",
    .description = "Returns fractional Microsoft Serial Time for specified hour, minute, and second.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_timeserial_register(void) {
    lang_desc_register(&g_timeserial_desc);
}

BValue func_timeserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "TIMESERIAL expects 3 numeric arguments (hour, minute, second)";
        return res;
    }
    int h = (int)args[0].as.number;
    int m = (int)args[1].as.number;
    int s = (int)args[2].as.number;
    double frac = (double)(h * 3600 + m * 60 + s) / 86400.0;
    res.as.number = frac;
    return res;
}
