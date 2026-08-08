/**
 * @file chr.c
 * @brief CHR$ single-character string generation function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CHR$ built-in string function, creating a 1-character reference-counted string from an ASCII code (0 to 255).
 *
 * 2. WHY IT EXISTS:
 * Provides standard ASCII character creation matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument code in range 0-255, allocates a 1-byte heap string using `str_create()`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_chr'. Includes "eval/functions/string/chr.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support extended codepages or Unicode code point conversion in UTF-8 mode.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * ASCII code range validation [0, 255] returning ERR_ILLEGAL_FUNCTION_CALL (error 5) for out-of-range codes.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing 1-character string or ERR_ILLEGAL_FUNCTION_CALL (error 5).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check character code range bounds checking (`code < 0 || code > 255`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Pure 7-bit ASCII console / 8-bit byte output.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/chr.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/chr.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_chr_register(void) {
    MicroLibMetadata meta = {
        .name = "CHR$",
        .category = "String Functions",
        .syntax = "CHR$(code)",
        .help_text = "Returns a 1-character string containing the character corresponding to ASCII code (0-255).",
        .error_codes = "Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_chr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "CHR$") != 0 && strcmp(uname, "CHR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "CHR$ expects one numeric argument";
        return res;
    }

    int code = (int)args[0].as.number;
    if (code < 0 || code > 255) {
        err->code = 5;
        err->message = "Illegal function call in CHR$";
        return res;
    }

    char ch = (char)code;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), &ch, 1);
    return res;
}
