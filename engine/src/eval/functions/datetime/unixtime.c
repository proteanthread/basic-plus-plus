// FILENAME: unixtime.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h)
// NEEDS: libengine (unixtime.h)
// Provides runtime implementation for the UNIXTIME function in BASIC++.

#include "eval/functions/datetime/unixtime.h"
#include "runtime/language_descriptor.h"
#include "hal/hal.h"
#include <stdint.h>

static const LangDesc g_unixtime_desc = {
    .name = "UNIXTIME",
    .category = "Date and Time",
    .syntax = "UNIXTIME()",
    .description = "Returns current Unix epoch timestamp in seconds.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_unixtime_register(void) {
    lang_desc_register(&g_unixtime_desc);
}

BValue func_unixtime_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    (void)arg_count;
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    int64_t now_sec = (hal_get() && hal_get()->time.now_epoch_seconds) ? hal_get()->time.now_epoch_seconds() : 0;
    res.as.number = (double)now_sec;
    return res;
}
