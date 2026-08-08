/**
 * @file date.c
 * @brief DATE$ / DATE system date function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements DATE$ (returns "MM-DD-YYYY" string) and DATE (returns YYYYMMDD numeric value) built-in system functions.
 *
 * 2. WHY IT EXISTS:
 * Provides standard date querying matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, queries system time via ISO C `time(NULL)`, formats using thread-safe `platform_localtime()`, and returns `VAL_STRING` (for `DATE$`) or `VAL_NUMBER` (for `DATE`).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_date'. Includes "eval/functions/system/date.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <string.h>, <time.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support custom date formatting options (e.g. YYYY-MM-DD vs MM/DD/YYYY).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING or VAL_NUMBER BValue containing current system date or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `snprintf` buffer sizing (at least 64 bytes for tm_year GCC warning compliance).
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
 * - engine/include/eval/functions/system/date.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/date.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void func_date_register(void) {
    MicroLibMetadata meta = {
        .name = "DATE$",
        .category = "System Functions",
        .syntax = "DATE$()",
        .help_text = "Returns the current system date as a string (MM-DD-YYYY).",
        .error_codes = "Error 5: Illegal Function Call (DATE$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_date_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "DATE / DATE$ expects no arguments";
        return res;
    }

    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);

    if (strcmp(uname, "DATE$") == 0) {
        char buf[32] = "";
        if (lt) {
            strftime(buf, sizeof(buf), "%m-%d-%Y", lt);
        }
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    } else {
        double val = 0.0;
        if (lt) {
            val = (lt->tm_year + 1900) * 10000.0 + (lt->tm_mon + 1) * 100.0 + lt->tm_mday;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}
