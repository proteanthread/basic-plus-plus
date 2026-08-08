/**
 * @file print.c
 * @brief PRINT statement execution handler and formatting evaluator for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements standard PRINT and PRINT USING statement evaluation, tab stop alignment (commas),
 * non-advancing concatenation (semicolons), numeric formatting (via num_format_display),
 * and file channel stream dispatch (PRINT #n, ...).
 *
 * 2. WHY IT EXISTS:
 * Provides essential screen, console, and stream output capabilities for BASIC++ across all
 * target execution profiles (baspp, bpp, bs).
 *
 * 3. WHY IT WORKS THIS WAY:
 * Iterates ephemerally over expression tokens on the current source line. Expressions are evaluated
 * via eval_expression(). Commas advance the vcon column position to 14-character tab stops
 * ((col / 14 + 1) * 14). Trailing punctuation suppresses the automatic trailing newline.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_print'. Directly includes "statements/core/print.h",
 * "device/vdev.h", "device/vcon.h", "eval/eval.h", "runtime/using.h", and "runtime/num_format.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in both libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To modify tab width, update the column calculation multiplier. To add custom formatting modifiers,
 * extend token inspection loops inside stmt_print_exec().
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Virtual device routing invariant: all screen output MUST go through vdev_puts/vcon_get_cursor.
 * String reference counting invariant: VAL_STRING values returned by eval_expression MUST be
 * freed with str_release() before function exit.
 *
 * 8. WHAT TO EXPECT:
 * Returns ERR_NONE on success, ERR_SYNTAX on malformed expressions, or ERR_TYPE_MISMATCH on type errors.
 * Positive numbers display with a leading space; all numbers display with a trailing space.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect column tracking via vcon_get_cursor(). Verify string refcounts to fix memory leaks.
 * Test token lookahead bounds using lex_peek().
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext. Active virtual console (CON:) or file handle context.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Pure 7-bit ASCII console output. Pointer safety via uintptr_t.
 * Safe buffer bounds sizing for snprintf formatting.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/device/vcon.c
 * - engine/src/device/vdev.c
 * - engine/src/eval/eval.c
 * - engine/src/runtime/num_format.c
 * - engine/src/runtime/using.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/print.h
 * - engine/include/device/vcon.h
 * - engine/include/device/vdev.h
 * - engine/include/eval/eval.h
 * - engine/include/runtime/num_format.h
 * - engine/include/runtime/using.h
 */

#include "stmt/stmt.h"
#include "device/vdev.h"
#include "device/vcon.h"
#include "eval/eval.h"
#include "runtime/using.h"
#include "runtime/num_format.h"
#include "runtime/micro_lib_metadata.h"

void stmt_print_register(void) {
    MicroLibMetadata meta = {
        .name = "PRINT",
        .category = "Console I/O",
        .syntax = "PRINT [#n,] [exprlist] [;|,]",
        .help_text = "Outputs formatted text or numeric expressions to the console or open file channel.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}
#include <stdio.h>
#include <string.h>
#include <math.h>

BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex);
BppError stmt_print_using_handler(VMContext *vm, LexerContext *lex);
void print_using_internal_ex(VMContext *vm, LexerContext *lex, int channel, FILE *stream);
void print_using_internal(VMContext *vm, LexerContext *lex, int channel);

int g_lpos = 1;

static void printer_write_str(FILE *fp, const char *s) {
    if (!s) return;
    while (*s) {
        fputc(*s, fp);
        if (*s == '\n' || *s == '\r') {
            g_lpos = 1;
        } else if (*s == '\t') {
            g_lpos = ((g_lpos - 1 + 8) / 8) * 8 + 1;
        } else {
            g_lpos++;
        }
        s++;
    }
}

static void format_double_clean(char *buf, size_t buf_size, double val, bool leading_space, bool trailing_space) {
    num_format_display(buf, buf_size, val, leading_space, trailing_space);
}

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
    /* Bug #19: Initialize col from actual console cursor position rather than
     * always 0 so TAB/comma spacing works correctly after trailing semicolons */
    int init_col = 0;
    VConContext *vcon = vm_get_vcon(vm);
    if (vcon) {
        vcon_get_cursor(vcon, 0, NULL, &init_col);
    }
    size_t col = (init_col >= 0) ? (size_t)init_col : 0;

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
            format_double_clean(buf, sizeof(buf), num, true, true);
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

BppError stmt_lprint_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_using = false;
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        is_using = true;
    }

    FILE *fp = fopen("lpt1.txt", "a");
    if (!fp) {
        err.code = 5; err.message = "Failed to open printer device (lpt1.txt)";
        return err;
    }

    if (is_using) {
        print_using_internal_ex(vm, lex, -1, fp);
    } else {
        bool last_was_sep = false;
        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
                break;
            }

            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                printer_write_str(fp, "              ");
                last_was_sep = true;
                continue;
            }
            if (tok.type == TOK_SEMICOLON) {
                lex_next(lex);
                last_was_sep = true;
                continue;
            }

            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) { fclose(fp); return err; }

            if (val.type == VAL_STRING) {
                printer_write_str(fp, str_data(val.as.string));
                str_release(vm_get_str(vm), val.as.string);
            } else {
                char nbuf[64];
                format_double_clean(nbuf, sizeof(nbuf), val.as.number, true, true);
                printer_write_str(fp, nbuf);
            }
            last_was_sep = false;
        }
        if (!last_was_sep) {
            printer_write_str(fp, "\n");
        }
    }

    fclose(fp);
    return err;
}

BppError stmt_display_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_using = false;
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        is_using = true;
    }

    VDevContext *vdev = vm_get_vdev(vm);

    if (is_using) {
        print_using_internal(vm, lex, -1);
    } else {
        bool last_was_sep = false;
        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
                break;
            }

            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                vdev_puts(vdev, "          ");
                last_was_sep = true;
                continue;
            }
            if (tok.type == TOK_SEMICOLON) {
                lex_next(lex);
                last_was_sep = true;
                continue;
            }

            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            if (val.type == VAL_STRING) {
                vdev_puts(vdev, str_data(val.as.string));
                str_release(vm_get_str(vm), val.as.string);
            } else {
                char buf[64];
                num_format_serialize(buf, sizeof(buf), val.as.number);
                vdev_puts(vdev, " ");
                vdev_puts(vdev, buf);
                vdev_puts(vdev, " ");
            }
            last_was_sep = false;
        }
        if (!last_was_sep) {
            vdev_putc(vdev, '\n');
        }
    }

    return err;
}
