// FILENAME: epochdate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h)
// NEEDS: libengine (epochdate.h)
// Provides runtime implementation for the EPOCHDATE function in BASIC++.

#include "eval/functions/datetime/epochdate.h"
#include "runtime/language_descriptor.h"
#include "hal/hal.h"
#include <stdint.h>

static const LangDesc g_epochdate_desc = {
    .name = "EPOCHDATE",
    .category = "Date and Time",
    .syntax = "EPOCHDATE([serial#])",
    .description = "Converts Microsoft Serial Date to Unix epoch seconds or returns current timestamp.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_epochdate_register(void) {
    lang_desc_register(&g_epochdate_desc);
}

BValue func_epochdate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) {
        int64_t now_sec = (hal_get() && hal_get()->time.now_epoch_seconds) ? hal_get()->time.now_epoch_seconds() : 0;
        res.as.number = (double)now_sec;
    } else {
        double ms_serial = args[0].as.number;
        // Convert MS Serial Date to Unix timestamp
        res.as.number = (ms_serial - 25569.0) * 86400.0;
    }
    return res;
}
