// FILENAME: hour.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (datetime_common.h, hour.h)
// Provides runtime implementation for the HOUR function in BASIC++.

#include "eval/functions/datetime/hour.h"
#include "eval/functions/datetime/datetime_common.h"
#include "runtime/language_descriptor.h"

#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_hour_desc = {
    .name = "HOUR",
    .category = "Date and Time",
    .syntax = "HOUR[(serial#)] / HOURS / HOUR$",
    .description = "Returns hour of the day in 24-hr (HOUR) or 12-hr (HOURS) or formatted with AM/PM (HOUR$).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_hour_register(void) {
    lang_desc_register(&g_hour_desc);
}

BValue func_hour_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)err;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    int h = 0;
    if (arg_count < 1) {
        struct tm tm_b;
        struct tm *lt = get_current_local_tm(&tm_b);
        h = lt ? lt->tm_hour : 0;
    } else {
        serial_to_hms(get_serial_arg(args), &h, NULL, NULL);
    }

    if (uname && runtime_strcmp(uname, "HOUR$") == 0) {
        int h12 = (h % 12 == 0 ? 12 : h % 12);
        char buf[16];
        runtime_snprintf(buf, sizeof(buf), "%02d %s", h12, (h >= 12 ? "PM" : "AM"));
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }
    if (uname && runtime_strcmp(uname, "HOURS") == 0) {
        int h12 = (h % 12 == 0 ? 12 : h % 12);
        res.as.number = (double)h12;
        return res;
    }
    res.as.number = (double)h;
    return res;
}
