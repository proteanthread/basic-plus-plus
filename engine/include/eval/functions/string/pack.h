/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file pack.h
 * @brief Public interface header for PACK$ binary structure packing function evaluator micro-library.
 *
 * 1. WHAT IT DOES:
 * Declares public entry point `func_pack_eval()` and metadata registration `func_pack_register()` for PACK$ function.
 *
 * 2. WHY IT EXISTS:
 * Exposes API interface for PACK$ function evaluation and online help catalog registration.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard function evaluator signature `BValue func_*_eval(VMContext*, const char*, int, BValue*, BppError*)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'func_pack'. Includes "vm/vm.h", "eval/eval.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add binary structure packing inline macro helpers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function prototypes func_pack_eval() and func_pack_register().
 *
 * 8. WHAT TO EXPECT:
 * Declares BValue return types and void registration entry points.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard EVAL_FUNC_PACK_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/functions/string/pack.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/include/eval/eval.h
 */

#ifndef EVAL_FUNC_PACK_H
#define EVAL_FUNC_PACK_H

#include "vm/vm.h"
#include "eval/eval.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Evaluates the PACK$() serialization built-in function.
 * @param vm Pointer to active VM context.
 * @param uname Uppercase function name.
 * @param arg_count Number of passed arguments.
 * @param args Array of argument values.
 * @param err Error state output parameter.
 * @return BValue holding serialized string.
 */
BValue func_pack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_pack_register(void);

#ifdef __cplusplus
}
#endif

#endif /* EVAL_FUNC_PACK_H */
