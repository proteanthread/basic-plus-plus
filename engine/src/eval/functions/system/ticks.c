/**
 * @file ticks.c
 * @brief TICKS / TI$ uptime function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements TICKS() (returns system uptime in milliseconds) and TI$ (returns formatted "HHMMSS" uptime string) system functions.
 *
 * 2. WHY IT EXISTS:
 * Provides Commodore PET / BASIC 2.0 / GW-BASIC timer and uptime querying parity.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, reads high-resolution system uptime via `platform_get_uptime()`, adjusts by `vm_get_ti_offset(vm)`, and formats or returns value.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_ticks'. Includes "eval/functions/system/ticks.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support microsecond resolution uptime querying (`MICROSECS()`).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument signature validation requirement.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING or VAL_NUMBER BValue containing uptime or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check platform high-resolution timer in `platform/platform.c`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses platform timer abstraction `platform_get_uptime()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/ticks.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/ticks.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include "runtime/strings.h"
#include <stdio.h>
#include <string.h>

void func_ticks_register(void) {
    MicroLibMetadata meta = {
        .name = "TICKS",
        .category = "System Functions",
        .syntax = "TICKS()",
        .help_text = "Returns the number of milliseconds elapsed since system start / platform boot.",
        .error_codes = "Error 5: Illegal Function Call (TICKS expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_ticks_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 0) {
        err->code = 13;
        err->message = "TI$ expects no arguments";
        return res;
    }

    double adjusted = platform_get_uptime() + vm_get_ti_offset(vm);
    if (adjusted < 0.0) adjusted = 0.0;

    long long sec_total = (long long)adjusted;
    long long days = sec_total / 86400;
    long long remaining = sec_total % 86400;
    int hr = (int)(remaining / 3600);
    int min = (int)((remaining / 60) % 60);
    int sec = (int)(remaining % 60);

    char buf[64];
    if (days == 0) {
        snprintf(buf, sizeof(buf), "%02d%02d%02d", hr, min, sec);
    } else {
        snprintf(buf, sizeof(buf), "%03lld:%02d%02d%02d", days, hr, min, sec);
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    return res;
}
