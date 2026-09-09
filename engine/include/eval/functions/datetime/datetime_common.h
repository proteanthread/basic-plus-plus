// FILENAME: datetime_common.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dateserial.c, timeserial.c, datevalue.c, timevalue.c, day.c, month.c, year.c, weekday.c, hour.c, minute.c, second.c)
// NEEDS: libcore (math.h, types.h)
// Provides shared date and time serialization algorithms for BASIC++.

#ifndef DATETIME_COMMON_H
#define DATETIME_COMMON_H

#include "runtime/math.h"
#include "types/types.h"
#include "eval/eval.h"
#include <stdint.h>
#include "runtime/math/math.h"

#define MS_SERIAL_EPOCH_JD 2415018.5

// Convert Year, Month, Day to Microsoft Serial Date (days since Dec 30, 1899)
static inline double ymd_to_serial(int year, int month, int day) {
    if (year < 100) {
        year += (year < 30) ? 2000 : 1900;
    }
    int y = year;
    int m = month;
    if (m <= 2) {
        y -= 1;
        m += 12;
    }
    int a = y / 100;
    int b = 2 - a + (a / 4);
    double jd = runtime_floor(365.25 * (y + 4716)) + runtime_floor(30.6001 * (m + 1)) + day + b - 1524.5;
    return jd - MS_SERIAL_EPOCH_JD;
}

// Decompose Microsoft Serial Date into Year, Month, Day, Weekday
static inline void serial_to_ymd(double serial, int *out_year, int *out_month, int *out_day, int *out_weekday) {
    double jd = serial + MS_SERIAL_EPOCH_JD;
    long z = (long)runtime_floor(jd + 0.5);
    
    long a = z;
    if (z >= 2299161) {
        long alpha = (long)runtime_floor((z - 1867216.25) / 36524.25);
        a = z + 1 + alpha - (alpha / 4);
    }
    long b = a + 1524;
    long c = (long)runtime_floor((b - 122.1) / 365.25);
    long d = (long)runtime_floor(365.25 * c);
    long e = (long)runtime_floor((b - d) / 30.6001);
    
    int day = (int)(b - d - (long)runtime_floor(30.6001 * e));
    int month = (e < 14) ? (int)(e - 1) : (int)(e - 13);
    int year = (month > 2) ? (int)(c - 4716) : (int)(c - 4715);
    
    if (out_year) *out_year = year;
    if (out_month) *out_month = month;
    if (out_day) *out_day = day;
    if (out_weekday) {
        int wd = (int)((z + 1) % 7) + 1; // 1 = Sunday, 2 = Monday, ... 7 = Saturday
        if (wd <= 0) wd += 7;
        *out_weekday = wd;
    }
}

// Decompose Serial Time into Hour, Minute, Second
static inline void serial_to_hms(double serial, int *out_hour, int *out_minute, int *out_second) {
    double frac = serial - runtime_floor(serial);
    if (frac < 0.0) frac += 1.0;
    long total_sec = (long)runtime_floor(frac * 86400.0 + 0.5);
    if (total_sec >= 86400) total_sec = 86399;
    
    int h = (int)(total_sec / 3600);
    int m = (int)((total_sec % 3600) / 60);
    int s = (int)(total_sec % 60);
    
    if (out_hour) *out_hour = h;
    if (out_minute) *out_minute = m;
    if (out_second) *out_second = s;
}

#include "hal/hal.h"
#include "platform/platform.h"

static inline struct tm *get_current_local_tm(struct tm *out_tm) {
    time_t raw_t = (time_t)(hal_get() && hal_get()->time.now_epoch_seconds ? hal_get()->time.now_epoch_seconds() : 0);
    struct tm *lt = platform_localtime(&raw_t, out_tm);
    if (!lt) {
        out_tm->tm_hour = 12; out_tm->tm_min = 0; out_tm->tm_sec = 0;
        out_tm->tm_year = 126; out_tm->tm_mon = 8; out_tm->tm_mday = 1;
        out_tm->tm_wday = 2;
        return out_tm;
    }
    return lt;
}

static inline double get_serial_arg(BValue *args) {
    if (!args) return 0.0;
    if (args[0].type == VAL_STRING) {
        return 0.0;
    }
    return args[0].as.number;
}

#endif // DATETIME_COMMON_H
