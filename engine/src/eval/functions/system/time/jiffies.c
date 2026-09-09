// FILENAME: jiffies.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (jiffies.h)
// Provides runtime implementation and LanguageDescriptor for the JIFFIES system variable in BASIC++.

#include "eval/functions/system/time/jiffies.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"

static const LangDesc g_jiffies_desc = {
    .name = "JIFFIES",
    .category = "System Functions",
    .syntax = "JIFFIES",
    .description = "Returns the number of elapsed 50 Hz timer jiffies (1/50th second increments) since system boot.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_jiffies_register(void) {
    lang_desc_register(&g_jiffies_desc);
}

BValue func_jiffies_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    res.as.number = (double)(uint64_t)(platform_get_uptime() * 50.0);
    return res;
}
