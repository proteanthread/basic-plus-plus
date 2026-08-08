/**
 * @file time.c
 * @brief TIME$ / TIME system time function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements TIME$ (returns "HH:MM:SS" string) and TIME (returns numeric HHMMSS value) built-in system functions.
 *
 * 2. WHY IT EXISTS:
 * Provides standard time querying matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, queries real-time clock via `time(NULL)`, formats via `platform_localtime()`, and returns `VAL_STRING` or `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_time'. Includes "eval/functions/system/time.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <string.h>, <time.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support 12-hour AM/PM vs 24-hour time formatting configuration.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation requirement.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING or VAL_NUMBER BValue containing current time or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `snprintf` buffer sizing (at least 64 bytes for tm_year/tm_hour GCC truncation warning compliance).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid system real-time clock.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Thread-safe ISO C `time()` and `platform_localtime()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/time.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/time.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void func_time_register(void) {
    MicroLibMetadata meta = {
        .name = "TIME$",
        .category = "System Functions",
        .syntax = "TIME$()",
        .help_text = "Returns the current system time as a string (HH:MM:SS).",
        .error_codes = "Error 5: Illegal Function Call (TIME$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_time_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "TIME / TIME$ expects no arguments";
        return res;
    }

    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    if (strcmp(uname, "TIME$") == 0) {
        char buf[64] = "";
        if (lt) {
            int hour12 = lt->tm_hour % 12;
            if (hour12 == 0) hour12 = 12;
            char ap = (lt->tm_hour >= 12) ? 'P' : 'A';
            snprintf(buf, sizeof(buf), "%02d:%02d:%02d %c", hour12, lt->tm_min, lt->tm_sec, ap);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    } else {
        double val = 0.0;
        if (lt) {
            val = lt->tm_hour * 10000.0 + lt->tm_min * 100.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}
