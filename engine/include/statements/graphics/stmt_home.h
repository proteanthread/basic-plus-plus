/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_home.h
 * @brief HOME [n] statement header for BASIC++.
 *
 * 1. WHAT IT DOES:
 *    Declares public prototypes for HOME statement registration (`stmt_home_register()`)
 *    and execution handling (`stmt_home_handler()`).
 *
 * 2. WHY IT EXISTS:
 *    Moves console text cursor to top-left corner (1, 1) without clearing the text buffer,
 *    and optionally updates text color attributes when parameter n (0-15) is passed.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Executes cursor repositioning on VConContext and emits ANSI escape codes (\033[H).
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `vm/vm.h`, `lexer/lexer.h`, `types/errors.h`
 *    - CMake Target: `stmt_home` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in `baspp`, `bpp`, and `bs`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To change default coordinates or bounds, modify `stmt_home_handler()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - Function signatures for `stmt_home_register` and `stmt_home_handler`.
 *
 * 8. WHAT TO EXPECT:
 *    - Cursor row/col set to 1, ANSI sequence \033[H emitted to stdout.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check VConContext pointer and terminal escape sequence parsing.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - VMContext and LexerContext initialized.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     Prerequisite Header Files:
 *     - engine/include/vm/vm.h
 *     - engine/include/lexer/lexer.h
 *     - engine/include/types/errors.h
 */

#ifndef STATEMENTS_GRAPHICS_STMT_HOME_H
#define STATEMENTS_GRAPHICS_STMT_HOME_H

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "types/errors.h"

void     stmt_home_register(void);
BppError stmt_home_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_GRAPHICS_STMT_HOME_H */
