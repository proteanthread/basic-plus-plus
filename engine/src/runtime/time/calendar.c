// FILENAME: calendar.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c, eval_internal.h, vm_internal.h)
// NEEDED BY: libhardware (bios_int1a.c)
// NEEDED BY: libstandard (edit_render.c)
// NEEDS: libcore (calendar.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// Freestanding calendar date and day calculation implementation.
//
// ---- Includes ----

#include "runtime/time/calendar.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const int days_per_month[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

bool runtime_time_is_leap_year(int year) {
    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

int runtime_time_days_in_month(int year, int mon) {
    if (mon < 0 || mon > 11) return 0;
    int leap = runtime_time_is_leap_year(year) ? 1 : 0;
    return days_per_month[leap][mon];
}

bool runtime_time_epoch_to_calendar(int64_t epoch_seconds, RuntimeCalendarTime *out_cal) {
    if (!out_cal) return false;

    // Seconds and minutes
    int64_t days = epoch_seconds / 86400;
    int64_t rem_sec = epoch_seconds % 86400;
    if (rem_sec < 0) {
        rem_sec += 86400;
        days -= 1;
    }

    out_cal->hour = (int)(rem_sec / 3600);
    int rem_min = (int)(rem_sec % 3600);
    out_cal->min = rem_min / 60;
    out_cal->sec = rem_min % 60;
    out_cal->isdst = 0;

    // Day of week (1970-01-01 was Thursday = 4)
    int wday = (int)((days + 4) % 7);
    if (wday < 0) wday += 7;
    out_cal->wday = wday;

    // Year calculation (400-year Gregorian cycle)
    int year = 1970;
    while (days < 0) {
        year -= 1;
        days += runtime_time_is_leap_year(year) ? 366 : 365;
    }

    for (;;) {
        int year_days = runtime_time_is_leap_year(year) ? 366 : 365;
        if (days < year_days) {
            break;
        }
        days -= year_days;
        year++;
    }

    out_cal->year = year;
    out_cal->yday = (int)days;

    // Month calculation
    int leap = runtime_time_is_leap_year(year) ? 1 : 0;
    int mon = 0;
    while (mon < 12 && days >= days_per_month[leap][mon]) {
        days -= days_per_month[leap][mon];
        mon++;
    }

    out_cal->mon = mon;
    out_cal->mday = (int)days + 1;

    return true;
}

int64_t runtime_time_calendar_to_epoch(const RuntimeCalendarTime *cal) {
    if (!cal) return 0;

    int year = cal->year;
    int mon = cal->mon;
    int mday = cal->mday;

    int64_t days = 0;
    if (year >= 1970) {
        for (int y = 1970; y < year; ++y) {
            days += runtime_time_is_leap_year(y) ? 366 : 365;
        }
    } else {
        for (int y = year; y < 1970; ++y) {
            days -= runtime_time_is_leap_year(y) ? 366 : 365;
        }
    }

    int leap = runtime_time_is_leap_year(year) ? 1 : 0;
    for (int m = 0; m < mon && m < 12; ++m) {
        days += days_per_month[leap][m];
    }
    days += (mday - 1);

    int64_t epoch = days * 86400 + cal->hour * 3600 + cal->min * 60 + cal->sec;
    return epoch;
}

size_t runtime_time_format(char *s, size_t maxsize, const char *format, const RuntimeCalendarTime *timeptr) {
    if (!s || maxsize == 0 || !format || !timeptr) return 0;

    static const char *const ab_wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *const full_wday[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    static const char *const ab_mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    static const char *const full_mon[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    char *out = s;
    const char *end = s + maxsize;

    while (*format && out < end - 1) {
        if (*format != '%') {
            *out++ = *format++;
            continue;
        }

        format++; // skip '%'
        char spec = *format++;
        char temp[64];
        size_t tlen = 0;

        switch (spec) {
            case 'Y': tlen = runtime_snprintf(temp, sizeof(temp), "%04d", timeptr->year); break;
            case 'y': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->year % 100); break;
            case 'm': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->mon + 1); break;
            case 'B': tlen = runtime_strlcpy(temp, full_mon[timeptr->mon % 12], sizeof(temp)); break;
            case 'b':
            case 'h': tlen = runtime_strlcpy(temp, ab_mon[timeptr->mon % 12], sizeof(temp)); break;
            case 'd': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->mday); break;
            case 'e': tlen = runtime_snprintf(temp, sizeof(temp), "%2d", timeptr->mday); break;
            case 'H': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->hour); break;
            case 'I': {
                int h12 = timeptr->hour % 12;
                if (h12 == 0) h12 = 12;
                tlen = runtime_snprintf(temp, sizeof(temp), "%02d", h12);
                break;
            }
            case 'M': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->min); break;
            case 'S': tlen = runtime_snprintf(temp, sizeof(temp), "%02d", timeptr->sec); break;
            case 'p': tlen = runtime_strlcpy(temp, (timeptr->hour >= 12) ? "PM" : "AM", sizeof(temp)); break;
            case 'A': tlen = runtime_strlcpy(temp, full_wday[timeptr->wday % 7], sizeof(temp)); break;
            case 'a': tlen = runtime_strlcpy(temp, ab_wday[timeptr->wday % 7], sizeof(temp)); break;
            case 'w': tlen = runtime_snprintf(temp, sizeof(temp), "%d", timeptr->wday); break;
            case 'j': tlen = runtime_snprintf(temp, sizeof(temp), "%03d", timeptr->yday + 1); break;
            case '%': temp[0] = '%'; temp[1] = '\0'; tlen = 1; break;
            default:
                temp[0] = '%';
                temp[1] = spec;
                temp[2] = '\0';
                tlen = 2;
                break;
        }

        for (size_t i = 0; i < tlen && out < end - 1; ++i) {
            *out++ = temp[i];
        }
    }

    *out = '\0';
    return (size_t)(out - s);
}
