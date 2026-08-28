// FILENAME: pds_datetime.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (hal.h, math.h, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (math.c, pds_datetime.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PDS_DATETIME built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/datetime/pds_datetime.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "hal/hal.h"
#include "runtime/math.h"
#include "runtime/string.h"

#define MS_SERIAL_EPOCH_JD 2415018.5

// Convert Year, Month, Day to Microsoft Serial Date (days since Dec 30, 1899)
static double ymd_to_serial(int year, int month, int day) {
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
static void serial_to_ymd(double serial, int *out_year, int *out_month, int *out_day, int *out_weekday) {
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
static void serial_to_hms(double serial, int *out_hour, int *out_minute, int *out_second) {
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

BValue func_dateserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "DATESERIAL expects 3 numeric arguments (year, month, day)";
        return res;
    }
    int y = (int)args[0].as.number;
    int m = (int)args[1].as.number;
    int d = (int)args[2].as.number;
    res.as.number = ymd_to_serial(y, m, d);
    return res;
}

BValue func_timeserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 3) {
        err->code = 13; err->message = "TIMESERIAL expects 3 numeric arguments (hour, minute, second)";
        return res;
    }
    int h = (int)args[0].as.number;
    int m = (int)args[1].as.number;
    int s = (int)args[2].as.number;
    res.as.number = ((double)h * 3600.0 + (double)m * 60.0 + (double)s) / 86400.0;
    return res;
}

BValue func_datevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "DATEVALUE expects string argument";
        return res;
    }
    const char *s = str_data(args[0].as.string);
    int y = 1900, m = 1, d = 1;
    if (s && *s) {
        int v1 = 0, v2 = 0, v3 = 0;
        const char *p = s;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') { v1 = v1 * 10 + (*p - '0'); p++; }
        if (*p == '-' || *p == '/') {
            p++;
            while (*p >= '0' && *p <= '9') { v2 = v2 * 10 + (*p - '0'); p++; }
            if (*p == '-' || *p == '/') {
                p++;
                while (*p >= '0' && *p <= '9') { v3 = v3 * 10 + (*p - '0'); p++; }
            }
        }
        if (v1 > 1000) {
            // YYYY-MM-DD
            y = v1; m = v2; d = v3;
        } else if (v3 > 0) {
            // MM-DD-YY or MM/DD/YYYY
            m = v1; d = v2; y = v3;
        }
    }
    res.as.number = ymd_to_serial(y, m, d);
    return res;
}

BValue func_timevalue_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1 || args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "TIMEVALUE expects string argument";
        return res;
    }
    const char *s = str_data(args[0].as.string);
    int h = 0, m = 0, sec = 0;
    char ampm[8] = {0};
    if (s && *s) {
        const char *p = s;
        while (*p == ' ') p++;
        while (*p >= '0' && *p <= '9') { h = h * 10 + (*p - '0'); p++; }
        if (*p == ':') {
            p++;
            while (*p >= '0' && *p <= '9') { m = m * 10 + (*p - '0'); p++; }
            if (*p == ':') {
                p++;
                while (*p >= '0' && *p <= '9') { sec = sec * 10 + (*p - '0'); p++; }
            }
        }
        while (*p == ' ') p++;
        size_t ai = 0;
        while (*p && !runtime_isspace((unsigned char)*p) && ai < sizeof(ampm) - 1) {
            ampm[ai++] = *p++;
        }
        ampm[ai] = '\0';
        if (runtime_strcasecmp(ampm, "PM") == 0 && h < 12) h += 12;
        if (runtime_strcasecmp(ampm, "AM") == 0 && h == 12) h = 0;
    }
    res.as.number = ((double)h * 3600.0 + (double)m * 60.0 + (double)sec) / 86400.0;
    return res;
}

static double get_serial_arg(BValue *args) {
    double s = args[0].as.number;
    // If argument is a large Unix epoch timestamp (> 1000000), convert to MS Serial Date
    if (s > 1000000.0) {
        // Unix timestamp (seconds since Jan 1, 1970)
        // Jan 1, 1970 MS serial date is 25569.0
        s = 25569.0 + (s / 86400.0);
    }
    return s;
}

BValue func_day_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) {
        int64_t now_sec = (hal_get() && hal_get()->time.now_epoch_seconds) ? hal_get()->time.now_epoch_seconds() : 0;
        time_t t = (time_t)now_sec;
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.as.number = lt ? (double)lt->tm_mday : 1.0;
        return res;
    }
    int d = 0;
    serial_to_ymd(get_serial_arg(args), NULL, NULL, &d, NULL);
    res.as.number = (double)d;
    return res;
}

BValue func_month_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) {
        int64_t now_sec = (hal_get() && hal_get()->time.now_epoch_seconds) ? hal_get()->time.now_epoch_seconds() : 0;
        time_t t = (time_t)now_sec;
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.as.number = lt ? (double)(lt->tm_mon + 1) : 1.0;
        return res;
    }
    int m = 0;
    serial_to_ymd(get_serial_arg(args), NULL, &m, NULL, NULL);
    res.as.number = (double)m;
    return res;
}

BValue func_year_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) {
        int64_t now_sec = (hal_get() && hal_get()->time.now_epoch_seconds) ? hal_get()->time.now_epoch_seconds() : 0;
        time_t t = (time_t)now_sec;
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        res.as.number = lt ? (double)(lt->tm_year + 1900) : 2026.0;
        return res;
    }
    int y = 0;
    serial_to_ymd(get_serial_arg(args), &y, NULL, NULL, NULL);
    res.as.number = (double)y;
    return res;
}

BValue func_weekday_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) { err->code = 13; err->message = "WEEKDAY expects numeric serial argument"; return res; }
    int wd = 1;
    serial_to_ymd(get_serial_arg(args), NULL, NULL, NULL, &wd);
    res.as.number = (double)wd;
    return res;
}

BValue func_hour_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) { err->code = 13; err->message = "HOUR expects numeric serial argument"; return res; }
    int h = 0;
    serial_to_hms(get_serial_arg(args), &h, NULL, NULL);
    res.as.number = (double)h;
    return res;
}

BValue func_minute_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) { err->code = 13; err->message = "MINUTE expects numeric serial argument"; return res; }
    int m = 0;
    serial_to_hms(get_serial_arg(args), NULL, &m, NULL);
    res.as.number = (double)m;
    return res;
}

BValue func_second_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)vm;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (arg_count < 1) { err->code = 13; err->message = "SECOND expects numeric serial argument"; return res; }
    int s = 0;
    serial_to_hms(get_serial_arg(args), NULL, NULL, &s);
    res.as.number = (double)s;
    return res;
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

void func_pds_datetime_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PDS_DATETIME",
        .category = "Date and Time",
        .syntax = "DATESERIAL(y, m, d) / DAY(serial#) / WEEKDAY(serial#)"
    };
    microlib_register(&meta);
}

