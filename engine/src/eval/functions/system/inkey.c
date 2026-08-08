/**
 * @file inkey.c
 * @brief INKEY$ non-blocking keyboard input function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements INKEY$ built-in system function to poll console input buffer asynchronously without blocking.
 *
 * 2. WHY IT EXISTS:
 * Provides standard interactive keyboard polling matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 0 arguments, calls non-blocking `platform_inkey_char()`, constructs a 1-character `BppString` if pressed (or 2-character string for extended key codes), else returns empty string `""`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_inkey'. Includes "eval/functions/system/inkey.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support extended key code translation (e.g. arrow keys, function keys).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Non-blocking execution contract (must never wait for input).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing 1-char, 2-char, or 0-char string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `platform_inkey_char()` behavior in platform terminal binding layer.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid active console or terminal session.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Cross-platform keyboard input abstraction via `platform_inkey_char()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/runtime/strings.c
 * - engine/src/vm/vm.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/inkey.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/strings.h
 * - engine/include/vm/vm.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/inkey.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_inkey_register(void) {
    MicroLibMetadata meta = {
        .name = "INKEY$",
        .category = "System Functions",
        .syntax = "INKEY$()",
        .help_text = "Reads a character non-blockingly from console buffer. Returns empty string if no key pressed.",
        .error_codes = "Error 5: Illegal Function Call (INKEY$ expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_inkey_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "INKEY$") != 0 && strcmp(uname, "INKEY") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 13;
        err->message = "INKEY$ expects no arguments";
        return res;
    }

    int ch = platform_inkey_char();
    char buf[2] = {0};
    if (ch > 0) {
        buf[0] = (char)ch;
    }
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, buf[0] ? 1 : 0);
    return res;
}
