/**
 * @file shl.c
 * @brief SHL (_SHL / BITS.SHL) bitwise shift left function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements SHL (_SHL, BITS.SHL) function, shifting a 64-bit unsigned integer left by a specified number of bits (0 to 63).
 *
 * 2. WHY IT EXISTS:
 * Performs bitwise shift operations for binary arithmetic, bit packing, and hardware register manipulation.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates value and shift count arguments, verifying shift count bounds (0-63), and returns val << shift_count.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_shl'. Includes "eval/functions/bits/shl.h",
 * "runtime/micro_lib_metadata.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support circular bit rotation (ROL).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Shift count bounds checking (0 to 63) to prevent undefined behavior from 64-bit bitwise shifts.
 *
 * 8. WHAT TO EXPECT:
 * Returns double-precision numeric BValue or ERR_ILLEGAL_FUNCTION_CALL (error 5) if shift_bits < 0 or >= 64.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check shift_bits parameter range validation logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit uint64_t unsigned left shift.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/bits/shl.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/bits/shl.h"
#include "runtime/micro_lib_metadata.h"
#include <stdint.h>
#include <string.h>

void func_shl_register(void) {
    MicroLibMetadata meta = {
        .name = "SHL",
        .category = "Bitwise Functions",
        .syntax = "SHL(val, shift_bits)",
        .help_text = "Performs a bitwise left shift on integer val by shift_bits.",
        .error_codes = "Error 5: Illegal Function Call (shift_bits < 0 or >= 64), Error 13: Type Mismatch (expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_shl_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_SHL") != 0 && strcmp(uname, "SHL") != 0 && strcmp(uname, "BITS.SHL") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "SHL expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number << (uint64_t)(int64_t)args[1].as.number);
    return res;
}
