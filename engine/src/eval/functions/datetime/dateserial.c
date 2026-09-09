// FILENAME: dateserial.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, math.h)
// NEEDS: libengine (dateserial.h, datetime_common.h)
// Provides runtime implementation for the DATESERIAL function in BASIC++.

#include "eval/functions/datetime/dateserial.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_dateserial_desc = {
    .name = "DATESERIAL",
    .category = "Date and Time",
    .syntax = "DATESERIAL(year%, month%, day%)",
    .description = "Returns Microsoft Serial Date double-precision number for specified year, month, and day.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_dateserial_register(void) {
    lang_desc_register(&g_dateserial_desc);
}

BValue func_dateserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "DATESERIAL expects 3 numeric arguments (year, month, day)";
        return res;
    }
    int y = (int)args[0].as.number;
    int m = (int)args[1].as.number;
    int d = (int)args[2].as.number;
    res.as.number = ymd_to_serial(y, m, d);
    return res;
}
