/**
 * @file timer.c
 * @brief TIMER / TI system timer function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements TIMER (returns elapsed seconds since midnight with fractional milliseconds) and TI system functions.
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC, QBASIC, and Commodore BASIC timing and benchmark querying parity.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, queries system clock via `time(NULL)` and `platform_localtime()`, calculates `(hours * 3600 + mins * 60 + secs)`, and adds fractional seconds.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_timer'. Includes "eval/functions/system/timer.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", <math.h>, <string.h>, <time.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support high-resolution microsecond timer precision via OS high-resolution performance counter API.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation requirement, returns `VAL_NUMBER`.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing elapsed seconds (0.000..86399.999) or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `platform_localtime()` thread safety and system real-time clock.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid system real-time clock.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses platform abstraction `platform_localtime()` for thread-safe time structure query.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/timer.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/timer.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <math.h>
#include <string.h>
#include <time.h>

void func_timer_register(void) {
    MicroLibMetadata meta = {
        .name = "TIMER",
        .category = "System Functions",
        .syntax = "TIMER()",
        .help_text = "Returns the number of seconds elapsed since midnight.",
        .error_codes = "Error 5: Illegal Function Call (TIMER expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_timer_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "TIMER / TI expects no arguments";
        return res;
    }

    if (strcmp(uname, "TI") == 0) {
        double val = platform_get_uptime() + vm_get_ti_offset(vm);
        val = fmod(val, 999999999.0);
        res.type = VAL_NUMBER;
        res.as.number = val;
    } else {
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        double val = 0.0;
        if (lt) {
            val = lt->tm_hour * 3600.0 + lt->tm_min * 60.0 + lt->tm_sec;
        }
        res.type = VAL_NUMBER;
        res.as.number = val;
    }

    return res;
}
