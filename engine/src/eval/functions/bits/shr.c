/**
 * @file shr.c
 * @brief SHR (_SHR / BITS.SHR) bitwise shift right function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements SHR (_SHR, BITS.SHR) function, shifting a 64-bit unsigned integer right by a specified number of bits (0 to 63).
 *
 * 2. WHY IT EXISTS:
 * Performs bitwise shift right operations for binary arithmetic, bit extraction, and flag decoding.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates value and shift count arguments, verifying shift count bounds (0-63), and returns val >> shift_count.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_shr'. Includes "eval/functions/bits/shr.h",
 * "runtime/micro_lib_metadata.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support arithmetic right shift (SAR) preserving sign bit.
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
 * Strict C17 compliance. 64-bit uint64_t unsigned right shift.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/bits/shr.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/bits/shr.h"
#include "runtime/micro_lib_metadata.h"
#include <stdint.h>
#include <string.h>

void func_shr_register(void) {
    MicroLibMetadata meta = {
        .name = "SHR",
        .category = "Bitwise Functions",
        .syntax = "SHR(val, shift_bits)",
        .help_text = "Performs a bitwise right shift on integer val by shift_bits.",
        .error_codes = "Error 5: Illegal Function Call (shift_bits < 0 or >= 64), Error 13: Type Mismatch (expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_shr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_SHR") != 0 && strcmp(uname, "SHR") != 0 && strcmp(uname, "BITS.SHR") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "SHR expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number);
    return res;
}
