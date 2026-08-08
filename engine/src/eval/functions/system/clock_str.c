/**
 * @file clock_str.c
 * @brief CLOCK$ ISO date/time string function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CLOCK$ built-in string function, returning the current date and time as an ISO 8601 formatted string ("YYYY-MM-DD HH:MM:SS").
 *
 * 2. WHY IT EXISTS:
 * Provides standard ISO 8601 formatted date/time querying matching QBASIC and BASIC++ system library standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, queries system time via ISO C `time(NULL)`, formats via thread-safe `platform_gmtime()` / `platform_localtime()`, creates a `BppString`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_clock_str'. Includes "eval/functions/system/clock_str.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <time.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support custom strftime format parameters (e.g. CLOCK$("YYYY-MM-DD")).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing ISO 8601 string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `snprintf` buffer sizing (at least 64 bytes for tm_year GCC warning compliance).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid system real-time clock.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Thread-safe time handling.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/clock_str.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/clock_str.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

void func_clock_str_register(void) {
    MicroLibMetadata meta = {
        .name = "CLOCK$",
        .category = "System Functions",
        .syntax = "CLOCK$()",
        .help_text = "Returns the current ISO 8601 formatted date/time string (YYYY-MM-DD HH:MM:SS).",
        .error_codes = "Error 5: Illegal Function Call (CLOCK$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_clock_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "CLOCK$ expects no arguments";
        return res;
    }

    time_t t = time(NULL);
    struct tm tm_buf;
    extern struct tm *platform_gmtime(const time_t *timep, struct tm *result);
    struct tm *gt = platform_gmtime(&t, &tm_buf);

    char buf[64] = "";
    if (gt) {
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                 gt->tm_year + 1900, gt->tm_mon + 1, gt->tm_mday,
                 gt->tm_hour, gt->tm_min, gt->tm_sec);
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    return res;
}
