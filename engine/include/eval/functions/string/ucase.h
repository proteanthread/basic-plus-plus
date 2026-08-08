/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file ucase.h
 * @brief Public interface header for UCASE$ uppercase conversion function evaluator micro-library.
 *
 * 1. WHAT IT DOES:
 * Declares public entry point `func_ucase_eval()` and metadata registration `func_ucase_register()` for UCASE$ function.
 *
 * 2. WHY IT EXISTS:
 * Exposes API interface for UCASE$ function evaluation and online help catalog registration.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard function evaluator signature `BValue func_*_eval(VMContext*, const char*, int, BValue*, BppError*)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'func_ucase'. Includes "types/types.h", "eval/eval_internal.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add uppercase conversion inline macro helpers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function prototypes func_ucase_eval() and func_ucase_register().
 *
 * 8. WHAT TO EXPECT:
 * Declares BValue return types and void registration entry points.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard EVAL_FUNC_UCASE_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/functions/string/ucase.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/eval/eval_internal.h
 */

#ifndef EVAL_FUNC_UCASE_H
#define EVAL_FUNC_UCASE_H

#include "types/types.h"
#include "eval/eval_internal.h"

BValue func_ucase_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ucase_register(void);

#endif /* EVAL_FUNC_UCASE_H */
