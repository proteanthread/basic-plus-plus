// FILENAME: ti.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, string.h, format.h)
// NEEDS: libengine (ti.h)
// Provides runtime implementation and LanguageDescriptor for TI, TI$, and TIMER$ system variables in BASIC++.

#include "eval/functions/system/time/ti.h"
#include "runtime/language_descriptor.h"
#include "platform/platform.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "runtime/variables.h"

extern double vm_get_ti_offset(VMContext *vm);

static const LangDesc g_ti_desc = {
    .name = "TI",
    .category = "System Functions",
    .syntax = "TI",
    .description = "Returns the number of elapsed seconds since session/interpreter boot (with subsecond precision).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

static const LangDesc g_ti_str_desc = {
    .name = "TI$",
    .category = "System Functions",
    .syntax = "TI$",
    .description = "Returns formatted elapsed session duration as a 6-digit string (HHMMSS) or multi-day string (D days, HH:MM:SS).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

static const LangDesc g_timer_str_desc = {
    .name = "TIMER$",
    .category = "System Functions",
    .syntax = "TIMER$",
    .description = "Returns formatted elapsed session duration as a delimited string (HH:MM:SS or D days, HH:MM:SS).",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_ti_register(void) {
    lang_desc_register(&g_ti_desc);
    lang_desc_register(&g_ti_str_desc);
    lang_desc_register(&g_timer_str_desc);
}

BValue func_ti_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    (void)arg_count;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };

    double elapsed = platform_get_uptime() + vm_get_ti_offset(vm);
    if (elapsed < 0.0) elapsed = 0.0;

    if (uname && (runtime_strcmp(uname, "TI$") == 0 || runtime_strcmp(uname, "TIMER$") == 0)) {
        long long sec_total = (long long)elapsed;
        long long days = sec_total / 86400;
        long long rem = sec_total % 86400;
        int hr = (int)(rem / 3600);
        int min = (int)((rem / 60) % 60);
        int sec = (int)(rem % 60);

        char buf[64];
        if (runtime_strcmp(uname, "TI$") == 0) {
            if (days == 0) {
                runtime_snprintf(buf, sizeof(buf), "%02d%02d%02d", hr, min, sec);
            } else {
                runtime_snprintf(buf, sizeof(buf), "%lld days, %02d:%02d:%02d", days, hr, min, sec);
            }
        } else {
            if (days == 0) {
                runtime_snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hr, min, sec);
            } else {
                runtime_snprintf(buf, sizeof(buf), "%lld days, %02d:%02d:%02d", days, hr, min, sec);
            }
        }

        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = elapsed;
    return res;
}
