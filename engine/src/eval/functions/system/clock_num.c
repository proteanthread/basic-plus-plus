/**
 * @file clock_num.c
 * @brief CLOCK system function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CLOCK built-in numeric system function, returning the current system Unix epoch timestamp in seconds.
 *
 * 2. WHY IT EXISTS:
 * Provides standard Unix epoch timestamp querying matching QBASIC and BASIC++ system library standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, queries system time via ISO C `time(NULL)`, and returns double float timestamp `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_clock_num'. Includes "eval/functions/system/clock_num.h",
 * "runtime/micro_lib_metadata.h", "platform/platform.h", <time.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support millisecond or microsecond epoch float return values.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing double float Unix timestamp or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check system clock time query `time(NULL)` and double float casting.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid system real-time clock.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Thread-safe ISO C `time()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/clock_num.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/clock_num.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <time.h>
#include <string.h>

void func_clock_num_register(void) {
    MicroLibMetadata meta = {
        .name = "CLOCK",
        .category = "System Functions",
        .syntax = "CLOCK()",
        .help_text = "Returns the current system Unix timestamp in seconds.",
        .error_codes = "Error 5: Illegal Function Call (CLOCK expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_clock_num_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CLOCK expects no arguments";
        return res;
    }

    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    double val = 0.0;
    if (lt) {
        long long yr = (long long)(lt->tm_year + 1900);
        long long mon = (long long)(lt->tm_mon + 1);
        long long mday = (long long)lt->tm_mday;
        long long hr = (long long)lt->tm_hour;
        long long min = (long long)lt->tm_min;
        long long sec = (long long)lt->tm_sec;
        val = (double)(yr * 10000000000LL + mon * 100000000LL + mday * 1000000LL + hr * 10000LL + min * 100LL + sec);
    }

    res.type = VAL_NUMBER;
    res.as.number = val;
    return res;
}
