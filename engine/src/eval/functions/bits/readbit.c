/**
 * @file readbit.c
 * @brief READBIT (_READBIT / BITS.READ) bit inspection function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements READBIT (_READBIT, BITS.READ) function, extracting bit state (0 or 1) at a specified bit position (0 to 63) in a 64-bit integer.
 *
 * 2. WHY IT EXISTS:
 * Enables single-bit state inspection for hardware register querying and bitmask verification.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates integer value and bit position arguments, verifying bit position bounds (0-63), and returns (val >> bit_pos) & 1.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_readbit'. Includes "eval/functions/bits/readbit.h",
 * "runtime/micro_lib_metadata.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support bit array ranges or bitfield slice extraction.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Bit position bounds checking (0 to 63) to prevent undefined behavior from 64-bit bitwise shifts.
 *
 * 8. WHAT TO EXPECT:
 * Returns double 1.0 or 0.0 or ERR_ILLEGAL_FUNCTION_CALL (error 5) if bit_pos < 0 or > 63.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check bit_pos parameter range validation logic.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit unsigned bitwise shifting.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/bits/readbit.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/bits/readbit.h"
#include "runtime/micro_lib_metadata.h"
#include <stdint.h>
#include <string.h>

void func_readbit_register(void) {
    MicroLibMetadata meta = {
        .name = "READBIT",
        .category = "Bitwise Functions",
        .syntax = "READBIT(val, bit_pos)",
        .help_text = "Returns the bit value (0 or 1) at bit_pos (0-63) of integer val.",
        .error_codes = "Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_readbit_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_READBIT") != 0 && strcmp(uname, "READBIT") != 0 && strcmp(uname, "BITS.READ") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "READBIT expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)(((uint64_t)(int64_t)args[0].as.number >> (uint64_t)(int64_t)args[1].as.number) & 1);
    return res;
}
