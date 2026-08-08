/**
 * @file bitcount.c
 * @brief BITCOUNT (_BITCOUNT / BITS.COUNT) bit population count function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements BITCOUNT (_BITCOUNT, BITS.COUNT) function, calculating total number of set bits (population count / popcount) in a 64-bit integer.
 *
 * 2. WHY IT EXISTS:
 * Provides hardware bit population counting for bitfield manipulation and flag mask calculations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, casts to uint64_t, and computes set bit count via bitwise popcount.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_bitcount'. Includes "eval/functions/bits/bitcount.h",
 * "runtime/micro_lib_metadata.h", <stdint.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Use compiler popcount intrinsics (__builtin_popcountll / _mm_popcnt_u64) where hardware support is available.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 64-bit integer bit population count evaluation rules.
 *
 * 8. WHAT TO EXPECT:
 * Returns double-precision number BValue or ERR_TYPE_MISMATCH.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify 64-bit uint64_t integer conversion bounds.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Cross-platform uint64_t bitwise masking.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/bits/bitcount.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/bits/bitcount.h"
#include "runtime/micro_lib_metadata.h"
#include <stdint.h>
#include <string.h>

void func_bitcount_register(void) {
    MicroLibMetadata meta = {
        .name = "BITCOUNT",
        .category = "Bitwise Functions",
        .syntax = "BITCOUNT(x)",
        .help_text = "Returns the number of set bits (population count) in integer x.",
        .error_codes = "Error 13: Type Mismatch (BITCOUNT expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_bitcount_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_BITCOUNT") != 0 && strcmp(uname, "BITCOUNT") != 0 && strcmp(uname, "BITS.COUNT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "BITCOUNT expects one numeric argument";
        return res;
    }

    uint64_t temp = (uint64_t)(int64_t)args[0].as.number;
    int count = 0;
    while (temp) {
        if (temp & 1) count++;
        temp >>= 1;
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)count;
    return res;
}
