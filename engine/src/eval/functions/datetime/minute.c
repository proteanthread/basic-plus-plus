// FILENAME: minute.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, minute.h)
// Provides runtime implementation for the MINUTE function in BASIC++.

#include "eval/functions/datetime/minute.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_minute_desc = {
    .name = "MINUTE",
    .category = "Date and Time",
    .syntax = "MINUTE[(serial#)] / MINUTES / MINUTE$",
    .description = "Returns minute of the hour as integer (MINUTE), fractional (MINUTES), or formatted string (MINUTE$).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_minute_register(void) {
    lang_desc_register(&g_minute_desc);
}

BValue func_minute_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    int m = 0;
    int s = 0;
    if (arg_count < 1) {
        struct tm tm_b;
        struct tm *lt = get_current_local_tm(&tm_b);
        m = lt ? lt->tm_min : 0;
        s = lt ? lt->tm_sec : 0;
    } else {
        serial_to_hms(get_serial_arg(args), NULL, &m, &s);
    }

    if (uname && runtime_strcmp(uname, "MINUTE$") == 0) {
        char buf[16];
        runtime_snprintf(buf, sizeof(buf), "%02d", m);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }
    if (uname && runtime_strcmp(uname, "MINUTES") == 0) {
        if (arg_count < 1) {
            double timer_val = platform_get_timer();
            double sub_sec = timer_val - runtime_floor(timer_val);
            double mins = (double)m + ((double)s + sub_sec) / 60.0;
            res.as.number = runtime_round(mins * 10000.0) / 10000.0;
        } else {
            double mins = (double)m + ((double)s / 60.0);
            res.as.number = runtime_round(mins * 10000.0) / 10000.0;
        }
        return res;
    }
    res.as.number = (double)m;
    return res;
}
