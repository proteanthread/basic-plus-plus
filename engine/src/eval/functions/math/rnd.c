/**
 * @file rnd.c
 * @brief RND pseudo-random number generator function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements RND function, returning a double-precision pseudo-random float in half-open interval [0.0, 1.0).
 *
 * 2. WHY IT EXISTS:
 * Provides standard BASIC PRNG supporting seed initialization (x < 0), repeated value retrieval (x = 0), and standard generation (x > 0 or omitted).
 *
 * 3. WHY IT WORKS THIS WAY:
 * Evaluates optional seed argument `x`, updates `vm->last_rnd` state, and returns uniform random float in [0.0, 1.0).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_rnd'. Includes "eval/functions/math/rnd.h",
 * "runtime/micro_lib_metadata.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support cryptographically secure PRNG backends or Mersenne Twister engine.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Half-open range [0.0, 1.0) and QBASIC/GW-BASIC seed rules (RND(0) returns previous random number).
 *
 * 8. WHAT TO EXPECT:
 * Returns double float in range [0.0, 1.0).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check VM random seed state tracking (`vm->last_rnd`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext with active PRNG seed state.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <stdlib.h> `rand()` and `srand()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/rnd.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/rnd.h"
#include "runtime/micro_lib_metadata.h"
#include <stdlib.h>
#include <string.h>

void func_rnd_register(void) {
    MicroLibMetadata meta = {
        .name = "RND",
        .category = "Math Functions",
        .syntax = "RND[(x)]",
        .help_text = "Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator.",
        .error_codes = "Error 13: Type Mismatch (non-numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_rnd_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "RND") != 0) {
        return res;
    }

    res.type = VAL_NUMBER;
    double arg = 1.0;
    if (arg_count > 0 && args[0].type == VAL_NUMBER) {
        arg = args[0].as.number;
    }
    if (arg < 0.0) {
        srand((unsigned int)(-arg));
        double val = (double)rand() / (double)RAND_MAX;
        vm_set_last_rnd(vm, val);
        res.as.number = val;
    } else if (arg == 0.0) {
        res.as.number = vm_get_last_rnd(vm);
    } else {
        double val = (double)rand() / (double)RAND_MAX;
        vm_set_last_rnd(vm, val);
        res.as.number = val;
    }
    return res;
}
