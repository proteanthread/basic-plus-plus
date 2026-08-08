/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file lerp.h
 * @brief Public interface header for LERP linear interpolation function evaluator micro-library.
 *
 * 1. WHAT IT DOES:
 * Declares public entry point `func_lerp_eval()` and metadata registration `func_lerp_register()` for LERP function.
 *
 * 2. WHY IT EXISTS:
 * Exposes API interface for LERP function evaluation and online help catalog registration.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard function evaluator signature `BValue func_*_eval(VMContext*, const char*, int, BValue*, BppError*)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'func_lerp'. Includes "types/types.h", "eval/eval_internal.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add linear interpolation inline macro helpers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function prototypes func_lerp_eval() and func_lerp_register().
 *
 * 8. WHAT TO EXPECT:
 * Declares BValue return types and void registration entry points.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard EVAL_FUNC_LERP_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/functions/math/lerp.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/eval/eval_internal.h
 */

#ifndef EVAL_FUNC_LERP_H
#define EVAL_FUNC_LERP_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_lerp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_lerp_register(void);

#endif /* EVAL_FUNC_LERP_H */
