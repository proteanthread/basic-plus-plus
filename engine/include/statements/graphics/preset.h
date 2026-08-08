/**
 * @file preset.h
 * @brief Public interface header for PRESET pixel reset statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler function stmt_preset_handler() for PRESET statement execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to execute PRESET pixel operations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_preset' (libbasicpp ONLY). Includes "types/errors.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (SDL2 graphics subsystem). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add pixel plot helper function prototypes if extending vector drawing API.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_preset_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STMT_PRESET_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/graphics/preset.c
 * Prerequisite Header Files:
 * - engine/include/types/errors.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */
#ifndef STMT_PRESET_H
#define STMT_PRESET_H
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_preset_handler(VMContext *vm, LexerContext *lex);
#endif
