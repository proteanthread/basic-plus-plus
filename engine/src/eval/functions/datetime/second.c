// FILENAME: second.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, second.h)
// Provides runtime implementation for the SECOND function in BASIC++.

#include "eval/functions/datetime/second.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_second_desc = {
    .name = "SECOND",
    .category = "Date and Time",
    .syntax = "SECOND[(serial#)] / SECONDS / SECOND$",
    .description = "Returns second of the minute as integer (SECOND), 4-decimal precision (SECONDS), or formatted string (SECOND$).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_second_register(void) {
    lang_desc_register(&g_second_desc);
}

BValue func_second_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    int s = 0;
    if (arg_count < 1) {
        struct tm tm_b;
        struct tm *lt = get_current_local_tm(&tm_b);
        s = lt ? lt->tm_sec : 0;
    } else {
        serial_to_hms(get_serial_arg(args), NULL, NULL, &s);
    }

    if (uname && runtime_strcmp(uname, "SECOND$") == 0) {
        char buf[16];
        runtime_snprintf(buf, sizeof(buf), "%02d", s);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }
    if (uname && runtime_strcmp(uname, "SECONDS") == 0) {
        if (arg_count < 1) {
            double timer_val = platform_get_timer();
            double sub_sec = timer_val - runtime_floor(timer_val);
            double secs = (double)s + sub_sec;
            res.as.number = runtime_round(secs * 10000.0) / 10000.0;
        } else {
            res.as.number = (double)s;
        }
        return res;
    }
    res.as.number = (double)s;
    return res;
}
