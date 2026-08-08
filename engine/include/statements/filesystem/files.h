/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file files.h
 * @brief Public interface header for FILES directory listing statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_files_handler(), stmt_dir_handler(), stmt_pwd_handler(), stmt_path_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle FILES and DIR statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_files'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add directory filter options to API function parameters if extending directory query capabilities.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_files_handler, stmt_dir_handler, stmt_pwd_handler, stmt_path_handler.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_FILESYSTEM_FILES_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/filesystem/files.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_FILESYSTEM_FILES_H
#define STATEMENTS_FILESYSTEM_FILES_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_files_handler(VMContext *vm, LexerContext *lex);

BppError stmt_dir_handler(VMContext *vm, LexerContext *lex);

BppError stmt_pwd_handler(VMContext *vm, LexerContext *lex);

BppError stmt_path_handler(VMContext *vm, LexerContext *lex);

BppError stmt_unsave_handler(VMContext *vm, LexerContext *lex);

BppError stmt_scratch_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_FILESYSTEM_FILES_H */
