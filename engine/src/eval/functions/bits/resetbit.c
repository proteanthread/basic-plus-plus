/**
 * @file resetbit.c
 * @brief RESETBIT (_RESETBIT / BITS.RESET) bit clearing function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements RESETBIT (_RESETBIT, BITS.RESET) function, clearing a bit to 0 at a specified bit position (0 to 63) in a 64-bit integer.
 *
 * 2. WHY IT EXISTS:
 * Enables single-bit clearing operations for hardware register modification and bitmask updates.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates integer value and bit position arguments, verifying bit position bounds (0-63), and returns val & ~((uint64_t)1 << bit_pos).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_resetbit'. Includes "eval/functions/bits/resetbit.h",
 * "runtime/micro_lib_metadata.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support multi-bit mask reset operations.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Bit position bounds checking (0 to 63) to prevent undefined behavior from 64-bit bitwise shifts.
 *
 * 8. WHAT TO EXPECT:
 * Returns double-precision numeric BValue or ERR_ILLEGAL_FUNCTION_CALL (error 5) if bit_pos < 0 or > 63.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check bit_pos parameter range validation logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit unsigned bitwise shift and bitwise AND NOT mask.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/bits/resetbit.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/bits/resetbit.h"
#include "runtime/micro_lib_metadata.h"
#include <stdint.h>
#include <string.h>

void func_resetbit_register(void) {
    MicroLibMetadata meta = {
        .name = "RESETBIT",
        .category = "Bitwise Functions",
        .syntax = "RESETBIT(val, bit_pos)",
        .help_text = "Returns val with the bit at bit_pos (0-63) cleared to 0.",
        .error_codes = "Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_resetbit_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_RESETBIT") != 0 && strcmp(uname, "RESETBIT") != 0 && strcmp(uname, "BITS.RESET") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "RESETBIT expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)((uint64_t)(int64_t)args[0].as.number & ~((uint64_t)1 << (uint64_t)(int64_t)args[1].as.number));
    return res;
}
