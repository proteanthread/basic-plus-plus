// FILENAME: func_week.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: exec_dispatch.c, common_reg_funcs.c, eval_ident_builtin.c
// NEEDS: libkernel, libcore, libplatform
// Implementation for WEEK and WEEK$ date functions in BASIC++.
//
// ---- Includes ----

#include "eval/functions/datetime/func_week.h"
#include "hal/hal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/strings.h"

static const LangDesc g_week_desc = {
    .name = "WEEK",
    .category = "Clocks & Timers",
    .syntax = "WEEK or WEEK()",
    .description = "Returns current week number of the year (ww) as a numeric integer.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_week_str_desc = {
    .name = "WEEK$",
    .category = "Clocks & Timers",
    .syntax = "WEEK$ or WEEK$()",
    .description = "Returns formatted ISO date string without W ('YYYY-ww-d') with locale-aware day numbering.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_week_register(void) {
    lang_desc_register(&g_week_desc);
    lang_desc_register(&g_week_str_desc);
}

static void get_week_and_day(int *out_year, int *out_week, int *out_day) {
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : (int64_t)time(NULL));
    struct tm tm_b;
    struct tm *gt = platform_gmtime(&raw_t, &tm_b);
    if (!gt) {
        tm_b.tm_year = 126; tm_b.tm_mon = 8; tm_b.tm_mday = 2;
        tm_b.tm_wday = 3; tm_b.tm_yday = 244;
        gt = &tm_b;
    }

    int year = gt->tm_year + 1900;
    int yday = gt->tm_yday; // 0..365
    int wday = gt->tm_wday; // 0=Sun, 1=Mon, ..., 6=Sat

    // Check locale: if LANG$ or environment contains "ISO" or European format
    const char *lang = platform_getenv("LANG");
    bool iso_locale = (lang && (runtime_strstr(lang, "GB") || runtime_strstr(lang, "DE") ||
                                runtime_strstr(lang, "FR") || runtime_strstr(lang, "ISO")));

    int day_num = 0;
    int week_num = 0;

    if (iso_locale) {
        // ISO 8601: Monday=1, ..., Sunday=7
        day_num = (wday == 0 ? 7 : wday);
        int d4 = (yday + 1) + 4 - day_num;
        week_num = (d4 / 7) + 1;
    } else {
        // Standard US Locale (Default): Sunday=1, Monday=2, Tuesday=3, ..., Saturday=7
        day_num = wday + 1;
        // Week number from beginning of year
        int start_offset = (yday - wday + 7) % 7;
        week_num = ((yday + start_offset) / 7) + 1;
    }

    if (week_num < 1) week_num = 1;
    if (week_num > 53) week_num = 53;

    if (out_year) *out_year = year;
    if (out_week) *out_week = week_num;
    if (out_day)  *out_day  = day_num;
}

BValue func_week_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err) {
    (void)vm; (void)name; (void)argc; (void)argv;
    if (err) runtime_memset(err, 0, sizeof(*err));

    int year = 0, week = 0, day = 0;
    get_week_and_day(&year, &week, &day);

    BValue result;
    runtime_memset(&result, 0, sizeof(result));
    result.type = VAL_NUMBER;
    result.as.number = (double)week;
    return result;
}

BValue func_week_str_eval(VMContext *vm, const char *name, int argc, BValue *argv, BppError *err) {
    (void)name; (void)argc; (void)argv;
    if (err) runtime_memset(err, 0, sizeof(*err));

    int year = 0, week = 0, day = 0;
    get_week_and_day(&year, &week, &day);

    char buf[32];
    runtime_snprintf(buf, sizeof(buf), "%04d-%02d-%d", year, week, day);

    BValue result;
    runtime_memset(&result, 0, sizeof(result));
    result.type = VAL_STRING;
    result.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return result;
}
