/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file math.c
 * @brief VM random number generator seed state and mathematical engine helpers for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vm_rnd()`, `vm_randomize()`, and pseudo-random number generator state tracking.
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC and QBASIC RND and RANDOMIZE parity with seed state preservation.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maintains a Linear Congruential Generator (LCG) PRNG state in `vm->rnd_seed`, generating double-precision floating-point numbers in the range [0.0, 1.0).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "vm/vm.h", "vm_internal.h", <math.h>, <stdlib.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Swap LCG generator for Mersenne Twister or Xoshiro256** PRNG algorithm.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard `RND(x)` behavior: RND(0) returns previous random number, RND(negative) re-seeds generator deterministically.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float in [0.0, 1.0).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `vm->rnd_seed` initialization in `vm_create()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Integer overflow wrapping for LCG multiplication is well-defined unsigned arithmetic.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/context.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/src/vm/vm_internal.h
 */
#include "vm/vm.h"
#include "vm_internal.h"

double vm_get_last_rnd(VMContext *vm) { return vm ? vm->last_rnd : 0.0; }
void vm_set_last_rnd(VMContext *vm, double val) { if (vm) vm->last_rnd = val; }
