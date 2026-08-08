/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_home.c
 * @brief HOME [n] cursor top-left positioning statement handler implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 *    Moves the console text cursor to the top-left corner (1, 1) without clearing the text screen,
 *    and optionally updates the active text foreground color when n (0-15) is specified.
 *
 * 2. WHY IT EXISTS:
 *    Provides classic HOME cursor positioning compatible with Apple II and vintage BASIC dialects.
 *
 * 3. WHY IT WORKS THIS WAY:
 *    Repositions the cursor in VConContext, emits ANSI escape sequence \033[H, and updates foreground color.
 *
 * 4. DEPENDENCIES & COMPILATION:
 *    - Required Headers: `statements/graphics/stmt_home.h`, `types/errors.h`, `vm/vm.h`, `lexer/lexer.h`,
 *                        `eval/eval.h`, `device/vcon.h`, `runtime/micro_lib_metadata.h`
 *    - CMake Target: Part of `stmt_home` micro-library target in `engine/CMakeLists.txt`.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 *    - Included in `baspp`, `bpp`, and `bs`.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 *    - To adjust ANSI positioning sequences, update `stmt_home_handler()`.
 *
 * 7. WHAT CANNOT BE CHANGED:
 *    - Top-left 1-indexed (1, 1) target coordinate.
 *
 * 8. WHAT TO EXPECT:
 *    - Text cursor row/col set to 1, stdout output \033[H.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 *    - Verify VConContext initialization and terminal ANSI support.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 *     - VMContext and LexerContext initialized.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 *     - Strict C17 compliance (`-std=c17`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 *     Prerequisite Source Files:
 *     - engine/src/device/vcon.c
 *     Prerequisite Header Files:
 *     - engine/include/statements/graphics/stmt_home.h
 *     - engine/include/device/vcon.h
 *     - engine/include/eval/eval.h
 */

#include "statements/graphics/stmt_home.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vcon.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

static const int ansi_fg_map[16] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};

void stmt_home_register(void) {
    static const MicroLibMetadata meta = {
        .name = "HOME",
        .category = "Graphics & Display",
        .syntax = "HOME [n]",
        .help_text = "Moves text cursor to top-left corner (1, 1) without clearing text screen, optionally changing text color to n (0-15).",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_home_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    int color = -1;
    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_EOL && tok.type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (val.type == VAL_NUMBER) {
            color = (int)val.as.number;
            if (color < 0 || color > 15) {
                err.code = 5;
                err.message = "Illegal Function Call";
                return err;
            }
        } else {
            err.code = 5;
            err.message = "Illegal Function Call";
            return err;
        }
    }

    VConContext *vcon = vm_get_vcon(vm);
    if (vcon) {
        vcon_locate(vcon, 0, 1, 1);
    }

    if (color >= 0 && color <= 15) {
        if (vcon) {
            vcon_set_color(vcon, 0, color, -1);
        }
        printf("\033[%dm\033[H", ansi_fg_map[color]);
    } else {
        printf("\033[H");
    }
    fflush(stdout);

    return err;
}
