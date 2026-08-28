// FILENAME: calendar.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (calendar.c)
// NEEDED BY: libengine (context.c, eval_internal.h, vm_internal.h)
// NEEDED BY: libhardware (bios_int1a.c)
// NEEDED BY: libstandard (edit_render.c)
// NEEDS: platform, memory
// Freestanding civil calendar and timestamp conversion functions.
//
// ---- Includes ----

#ifndef RUNTIME_TIME_CALENDAR_H
#define RUNTIME_TIME_CALENDAR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Broken-down calendar time representation (freestanding struct tm equivalent)
typedef struct RuntimeCalendarTime {
    int sec;         // Seconds [0, 60] (allowing 1 leap second)
    int min;         // Minutes [0, 59]
    int hour;        // Hours [0, 23]
    int mday;        // Day of month [1, 31]
    int mon;         // Month [0, 11] (0 = January)
    int year;        // Year (e.g. 2026)
    int wday;        // Day of week [0, 6] (0 = Sunday)
    int yday;        // Day of year [0, 365] (0 = Jan 1)
    int isdst;       // Daylight Saving Time flag (>0 DST, 0 No DST, <0 Unknown)
} RuntimeCalendarTime;

// @brief Checks if a year is a leap year in the Gregorian calendar.
bool runtime_time_is_leap_year(int year);

// @brief Returns the number of days in a given month (0 = Jan, 11 = Dec) of a year.
int runtime_time_days_in_month(int year, int mon);

// @brief Converts Unix epoch timestamp (seconds since 1970-01-01 00:00:00 UTC) into UTC calendar time.
bool runtime_time_epoch_to_calendar(int64_t epoch_seconds, RuntimeCalendarTime *out_cal);

// @brief Converts UTC calendar time into Unix epoch timestamp.
int64_t runtime_time_calendar_to_epoch(const RuntimeCalendarTime *cal);

// @brief Formats calendar time according to standard format string (re-entrant, freestanding strftime subset).
size_t runtime_time_format(char *s, size_t maxsize, const char *format, const RuntimeCalendarTime *timeptr);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_TIME_CALENDAR_H
