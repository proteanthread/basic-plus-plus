/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file varptr.h
 * @brief Public interface header for VARPTR and VARPTR$ function evaluators.
 *
 * 1. WHAT IT DOES:
 * Declares function prototypes for VARPTR() and VARPTR$() memory pointer evaluators.
 *
 * 2. WHY IT EXISTS:
 * Provides memory address and descriptor pointer introspection for variables.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Queries pointer address via uintptr_t casting and formats integer / string descriptor headers.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_varptr'. Includes "types/types.h", "vm/vm.h", "eval/eval.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in desktop ('baspp') and REPL ('bpp') editions.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add descriptor metadata fields.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * BValue (*)(VMContext*, int, BValue*) evaluator signature.
 *
 * 8. WHAT TO EXPECT:
 * Evaluates memory address pointers safely.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check uintptr_t pointer safety cast compliance.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit pointer safe via uintptr_t.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/functions/varptr.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/eval/eval.h
 */

#ifndef FUNCTIONS_VARPTR_H
#define FUNCTIONS_VARPTR_H

#include "types/types.h"
#include "vm/vm.h"
#include "eval/eval.h"

BValue func_varptr_eval(BValue *args, int arg_count, void *rt);
BValue func_varptr_str_eval(BValue *args, int arg_count, void *rt);
void func_varptr_register(void);

#endif /* FUNCTIONS_VARPTR_H */
