/**
 * @file stmt_print.c
 * @brief PRINT statement command handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the PRINT statement. Evaluates multiple expressions separated
 *   by commas (tabulation alignment) or semicolons (concatenation) and writes to "CON:".
 * - Why it exists: Provides standard screen output capabilities for BASIC programs.
 * - Why it works this way: It iterates over tokens in the line. Expressions are evaluated
 *   using eval_expression. Commas advance the console output column to the next 14-char tab stop.
 *   If the statement does not end with a separator, it writes a newline.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Tab stop width (14 by default), spacing before/after numbers.
 * - What cannot be changed: Obligation to print through the virtual device system (CON:).
 * - What to expect: Numbers print with a leading space if positive, followed by a trailing space.
 * - What to do if something breaks: Trace token matches and ensure formatting outputs through vdev_printf.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: CON: device is registered and accessible.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add PRINT USING formatting or redirect to other devices (e.g. PRINT #1).
 * - How to write external extensions: Custom printing functions route characters through this vdev console interface.
 */

#include "bpp_stmt.h"
#include "bpp_vdev.h"
#include "bpp_eval.h"
#include <stdio.h>
#include <string.h>

BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_print_using_handler(VMContext *vm, LexerContext *lex);

BppError stmt_print_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); /* Consume '#' */
        return stmt_file_print_handler(vm, lex);
    }
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume 'USING' */
        return stmt_print_using_handler(vm, lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    StringContext *str_ctx = vm_get_str(vm);

    bool last_was_sep = false;
    size_t col = 0;

    tok = lex_peek(lex);
    while (tok.type != TOK_EOF && tok.type != TOK_EOL && (tok.type != TOK_KEYWORD || tok.as.keyword != KW_ELSE)) {
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            /* Tab stop to next multiple of 14 */
            size_t spaces = 14 - (col % 14);
            for (size_t i = 0; i < spaces; ++i) {
                vdev_putc(vdev, ' ');
            }
            last_was_sep = true;
            col += spaces;
            tok = lex_peek(lex);
            continue;
        }

        if (tok.type == TOK_SEMICOLON) {
            lex_next(lex);
            last_was_sep = true;
            tok = lex_peek(lex);
            continue;
        }

        /* Check for TAB or SPC print modifiers */
        if (tok.type == TOK_IDENT && tok.length == 3 && (strncasecmp(tok.start, "TAB", 3) == 0 || strncasecmp(tok.start, "SPC", 3) == 0)) {
            bool is_tab = (strncasecmp(tok.start, "TAB", 3) == 0);
            lex_next(lex); /* Consume TAB/SPC */
            BppToken open_paren = lex_peek(lex);
            if (open_paren.type != TOK_LPAREN) {
                err.code = 2; err.message = "Expected '(' after TAB/SPC";
                return err;
            }
            lex_next(lex); /* Consume '(' */
            BValue arg_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            BppToken close_paren = lex_peek(lex);
            if (close_paren.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after TAB/SPC argument";
                return err;
            }
            lex_next(lex); /* Consume ')' */

            int n = (int)arg_val.as.number;
            if (is_tab) {
                int spaces = n - (int)col;
                if (spaces > 0) {
                    for (int i = 0; i < spaces; ++i) {
                        vdev_putc(vdev, ' ');
                    }
                    col += spaces;
                }
            } else {
                if (n > 0) {
                    for (int i = 0; i < n; ++i) {
                        vdev_putc(vdev, ' ');
                    }
                    col += n;
                }
            }
            last_was_sep = false;
            tok = lex_peek(lex);
            continue;
        }

        /* Evaluate expression */
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            return err;
        }

        /* Print value */
        char buf[256];
        if (val.type == VAL_STRING) {
            const char *str_data_ptr = str_data(val.as.string);
            size_t len = strlen(str_data_ptr);
            vdev_puts(vdev, str_data_ptr);
            /* Release temporary string returned by expression */
            if (val.as.string) {
                str_release(str_ctx, val.as.string);
            }
            last_was_sep = false;
            col += len;
        } else {
            /* Number: positive prints with leading space, negative with minus. Trailing space follows. */
            double num = val.as.number;
            if (num >= 0.0) {
                snprintf(buf, sizeof(buf), " %g ", num);
            } else {
                snprintf(buf, sizeof(buf), "%g ", num);
            }
            size_t len = strlen(buf);
            vdev_puts(vdev, buf);
            last_was_sep = false;
            col += len;
        }

        tok = lex_peek(lex);
    }

    if (!last_was_sep) {
        vdev_putc(vdev, '\n');
    }

    return err;
}
