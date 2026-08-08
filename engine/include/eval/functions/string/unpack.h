/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file unpack.h
 * @brief Public interface header for UNPACK$ binary data decoding function evaluator micro-library.
 *
 * 1. WHAT IT DOES:
 * Declares public entry point `func_unpack_eval()` and metadata registration `func_unpack_register()` for UNPACK$ function.
 *
 * 2. WHY IT EXISTS:
 * Exposes API interface for UNPACK$ function evaluation and online help catalog registration.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard function evaluator signature `BValue func_*_eval(VMContext*, const char*, int, BValue*, BppError*)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'func_unpack'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add binary decoding inline macro helpers.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function prototypes func_unpack_eval() and func_unpack_register().
 *
 * 8. WHAT TO EXPECT:
 * Declares BValue return types and void registration entry points.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard EVAL_FUNCS_STRING_FUNC_UNPACK_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/functions/string/unpack.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef EVAL_FUNCS_STRING_FUNC_UNPACK_H
#define EVAL_FUNCS_STRING_FUNC_UNPACK_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BValue func_unpack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_unpack_register(void);
void   func_unpack_register(void);

#endif /* EVAL_FUNCS_STRING_FUNC_UNPACK_H */
