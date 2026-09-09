// FILENAME: hebrew.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (hebrew.h)
// Provides runtime implementation and LanguageDescriptor for HEBREW$ system variable in BASIC++.

#include "eval/functions/datetime/hebrew.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/math/math.h"
#include "hal/hal.h"
#include "platform/platform.h"

static const LangDesc g_hebrew_desc = {
    .name = "HEBREW$",
    .category = "Clocks & Timers",
    .syntax = "HEBREW$",
    .description = "Returns today's Hebrew calendar date formatted as a string (e.g. '19 Elul 5786').",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_VARIABLE
};

void func_hebrew_register(void) {
    lang_desc_register(&g_hebrew_desc);
}

static bool hebrew_is_leap(int h_year) {
    return ((7 * h_year + 1) % 19) < 7;
}

static int64_t hebrew_rosh_hashanah_jd(int h_year) {
    int64_t m = (235LL * (int64_t)h_year - 234LL) / 19LL;
    int64_t p = 204LL + 793LL * (m % 1080LL);
    int64_t h = 5LL + 12LL * m + 793LL * (m / 1080LL) + (p / 1080LL);
    p %= 1080LL;
    int64_t d = 1LL + 29LL * m + (h / 24LL);
    h %= 24LL;

    int dw = (int)((d + 1) % 7);
    if (h >= 18) {
        d++;
        dw = (int)((d + 1) % 7);
    }
    if (dw == 0 || dw == 3 || dw == 5) {
        d++;
        dw = (int)((d + 1) % 7);
    }
    if (!hebrew_is_leap(h_year) && dw == 2 && (h > 9 || (h == 9 && p >= 204))) {
        d += 2;
    } else if (hebrew_is_leap(h_year - 1) && dw == 1 && (h > 15 || (h == 15 && p >= 589))) {
        d += 1;
    }
    dw = (int)((d + 1) % 7);
    if (dw == 0 || dw == 3 || dw == 5) {
        d++;
    }
    return 347997LL + d;
}

static void jd_to_hebrew_date(int64_t jd, int *h_year, int *h_month, int *h_day) {
    int y = (int)((jd - 347997LL) / 365.2468) + 1;
    while (hebrew_rosh_hashanah_jd(y + 1) <= jd) y++;
    while (hebrew_rosh_hashanah_jd(y) > jd) y--;

    int64_t rh_cur = hebrew_rosh_hashanah_jd(y);
    int64_t rh_next = hebrew_rosh_hashanah_jd(y + 1);
    int year_len = (int)(rh_next - rh_cur);
    bool leap = hebrew_is_leap(y);

    int cheshvan_len = (year_len == 355 || year_len == 385) ? 30 : 29;
    int kislev_len = (year_len == 353 || year_len == 383) ? 29 : 30;

    int month_days[14];
    month_days[1] = 30; // Tishrei
    month_days[2] = cheshvan_len; // Cheshvan
    month_days[3] = kislev_len; // Kislev
    month_days[4] = 29; // Tevet
    month_days[5] = 30; // Shevat
    if (leap) {
        month_days[6] = 30; // Adar I
        month_days[7] = 29; // Adar II
        month_days[8] = 30; // Nisan
        month_days[9] = 29; // Iyar
        month_days[10] = 30; // Sivan
        month_days[11] = 29; // Tammuz
        month_days[12] = 30; // Av
        month_days[13] = 29; // Elul
    } else {
        month_days[6] = 29; // Adar
        month_days[7] = 30; // Nisan
        month_days[8] = 29; // Iyar
        month_days[9] = 30; // Sivan
        month_days[10] = 29; // Tammuz
        month_days[11] = 30; // Av
        month_days[12] = 29; // Elul
        month_days[13] = 0;
    }

    int day_of_year = (int)(jd - rh_cur + 1);
    int m = 1;
    int max_m = leap ? 13 : 12;
    while (m <= max_m && day_of_year > month_days[m]) {
        day_of_year -= month_days[m];
        m++;
    }
    *h_year = y;
    *h_month = m;
    *h_day = day_of_year;
}

static const char *get_hebrew_month_name(int h_year, int h_month) {
    bool leap = hebrew_is_leap(h_year);
    if (leap) {
        static const char *names[] = {
            "", "Tishrei", "Cheshvan", "Kislev", "Tevet", "Shevat",
            "Adar I", "Adar II", "Nisan", "Iyar", "Sivan", "Tammuz", "Av", "Elul"
        };
        return (h_month >= 1 && h_month <= 13) ? names[h_month] : "";
    } else {
        static const char *names[] = {
            "", "Tishrei", "Cheshvan", "Kislev", "Tevet", "Shevat",
            "Adar", "Nisan", "Iyar", "Sivan", "Tammuz", "Av", "Elul"
        };
        return (h_month >= 1 && h_month <= 12) ? names[h_month] : "";
    }
}

BValue func_hebrew_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    (void)err;
    (void)arg_count;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));

    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm tm_b;
    struct tm *lt = platform_localtime(&raw_t, &tm_b);
    if (!lt) {
        tm_b.tm_year = 126; tm_b.tm_mon = 8; tm_b.tm_mday = 1;
        lt = &tm_b;
    }

    int y = lt->tm_year + 1900;
    int m = lt->tm_mon + 1;
    int d = lt->tm_mday;

    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int64_t a = y / 100;
    int64_t b = 2 - a + (a / 4);
    int64_t jd = (int64_t)(365.25 * (y + 4716)) + (int64_t)(30.6001 * (m + 1)) + d + b - 1524;

    int hy = 0, hm = 0, hd = 0;
    jd_to_hebrew_date(jd, &hy, &hm, &hd);

    char buf[64];
    runtime_snprintf(buf, sizeof(buf), "%d %s %d", hd, get_hebrew_month_name(hy, hm), hy);
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
    return res;
}
