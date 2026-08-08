/**
 * @file cls.h
 * @brief Public interface header for CLS screen clear statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_cls_handler() for CLS statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle CLS statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_cls'. Includes "types/errors.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add clear mode enum definitions if expanding viewport parameters.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_cls_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STMT_CLS_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/cls.c
 * Prerequisite Header Files:
 * - engine/include/types/errors.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */
#ifndef STMT_CLS_H
#define STMT_CLS_H
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_cls_handler(VMContext *vm, LexerContext *lex);
#endif
