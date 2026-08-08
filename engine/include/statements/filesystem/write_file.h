/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file write_file.h
 * @brief Public interface header for WRITE #file_num CSV file write statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_write_file_handler() for WRITE # statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle WRITE # statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_write_file'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add CSV formatting configuration prototypes if making delimiters configurable.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_write_file_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_FILESYSTEM_WRITE_FILE_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/filesystem/write_file.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_FILESYSTEM_WRITE_FILE_H
#define STATEMENTS_FILESYSTEM_WRITE_FILE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_write_file_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_FILESYSTEM_WRITE_FILE_H */
