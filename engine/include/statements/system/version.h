/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file version.h
 * @brief Public interface header for VERSION statement and VER/VER$ evaluators.
 *
 * 1. WHAT IT DOES:
 * Declares function prototypes for the VERSION statement handler (tags program version)
 * and VER / VER$ evaluators (queries host engine version or tagged program version).
 *
 * 2. WHY IT EXISTS:
 * Enables BASIC++ programs to declare and query their own version numbers for debugging,
 * tracing, static analysis, and automated build pipelines.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Stores version string inside MemoryContext. VER$() returns engine version by default,
 * or VER$("PROGRAM") / VER$("APP") returns the program's tagged version.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_version'. Includes "types/types.h",
 * "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in desktop ('baspp') and REPL ('bpp') editions.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add additional version metadata parsing hooks.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard BppError (*)(VMContext*, LexerContext*) handler signature.
 *
 * 8. WHAT TO EXPECT:
 * Exposes stmt_version_handler(), func_ver_eval(), func_ver_str_eval(), and stmt_version_register().
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcount handling and memory manager context access.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Safe string copying.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/system/version.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_SYSTEM_VERSION_H
#define STATEMENTS_SYSTEM_VERSION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_version_handler(VMContext *vm, LexerContext *lex);
BppError stmt_ver_handler(VMContext *vm, LexerContext *lex);
BValue func_ver_str_eval(BValue *args, int arg_count, void *rt);
void stmt_version_register(void);

#endif /* STATEMENTS_SYSTEM_VERSION_H */
