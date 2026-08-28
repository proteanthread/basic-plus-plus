// FILENAME: print.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (dialect.h, dialect.c, math.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (num_format.h, num_format.c, string.h, using.h)
// NEEDS: libengine (eval.h, eval.c, math.c, stmt.h, string.c)
// NEEDS: libkernel (vcon.h, vcon.c, vdev.h, vdev.c)
// Provides runtime implementation for the PRINT statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "device/vdev.h"
#include "device/vcon.h"
#include "core/dialect.h"
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
        lex_next(lex); // Consume '#'
        return stmt_file_print_handler(vm, lex);
    }
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); // Consume 'USING'
        return stmt_print_using_handler(vm, lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    StringContext *str_ctx = vm_get_str(vm);

    bool last_was_sep = false;
    // Bug #19: Initialize col from actual console cursor position rather than
// always 0 so TAB/comma spacing works correctly after trailing semicolons
    int init_col = 0;
    VConContext *vcon = vm_get_vcon(vm);
    if (vcon) {
        vcon_get_cursor(vcon, 0, NULL, &init_col);
    }
    size_t col = (init_col >= 0) ? (size_t)init_col : 0;

    tok = lex_peek(lex);
    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_BACKSLASH && (tok.type != TOK_KEYWORD || tok.as.keyword != KW_ELSE)) {
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            size_t zone_width = vm ? (size_t)vm_get_zone_width(vm) : 14;
            if (zone_width == 0) zone_width = 14;
            size_t spaces = zone_width - (col % zone_width);
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

        // Check for TAB, SPC, SPA, or LIN print modifiers
        if (tok.type == TOK_IDENT && tok.length == 3 &&
            (strncasecmp(tok.start, "TAB", 3) == 0 || strncasecmp(tok.start, "SPC", 3) == 0 ||
             strncasecmp(tok.start, "SPA", 3) == 0 || strncasecmp(tok.start, "LIN", 3) == 0)) {
            bool is_tab = (strncasecmp(tok.start, "TAB", 3) == 0);
            bool is_lin = (strncasecmp(tok.start, "LIN", 3) == 0);
            lex_next(lex); // Consume modifier
            BppToken open_paren = lex_peek(lex);
            if (open_paren.type != TOK_LPAREN) {
                err.code = 2; err.message = "Expected '(' after print modifier";
                return err;
            }
            lex_next(lex); // Consume '('
            BValue arg_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            BppToken close_paren = lex_peek(lex);
            if (close_paren.type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after print modifier argument";
                return err;
            }
            lex_next(lex); // Consume ')'

            int n = (int)arg_val.as.number;
            if (is_lin) {
                if (n == 0) {
                    vdev_putc(vdev, '\r');
                    col = 0;
                } else if (n == -1) {
                    vdev_putc(vdev, '\f');
                    col = 0;
                } else if (n > 0) {
                    for (int i = 0; i < n; ++i) {
                        vdev_putc(vdev, '\n');
                    }
                    col = 0;
                }
            } else if (is_tab) {
                int target_col = (n < 1) ? 1 : n;
                int cur_col = (int)col + 1; // 1-based column
                if (target_col >= cur_col) {
                    int spaces = target_col - cur_col;
                    for (int i = 0; i < spaces; ++i) {
                        vdev_putc(vdev, ' ');
                    }
                    col += (size_t)spaces;
                } else {
                    vdev_putc(vdev, '\n');
                    col = 0;
                    int spaces = target_col - 1;
                    for (int i = 0; i < spaces; ++i) {
                        vdev_putc(vdev, ' ');
                    }
                    col = (size_t)spaces;
                }
            } else {
                // SPC or SPA
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

        // Check if next token begins a space-separated statement or block
        if (!last_was_sep) {
            if (tok.type == TOK_LPAREN || tok.type == TOK_LBRACKET) {
                break;
            }
            if (tok.type == TOK_KEYWORD) {
                BppKeywordId k = tok.as.keyword;
                if (k == KW_PRINT || k == KW_LET || k == KW_IF || k == KW_FOR || k == KW_WHILE ||
                    k == KW_UNTIL || k == KW_UNLESS || k == KW_GOTO || k == KW_GOSUB || k == KW_RETURN ||
                    k == KW_INPUT || k == KW_DIM || k == KW_CLS || k == KW_COLOR || k == KW_LOCATE ||
                    k == KW_BEEP || k == KW_SOUND || k == KW_PLAY || k == KW_END || k == KW_STOP ||
                    k == KW_REM || k == KW_LINE || k == KW_CIRCLE || k == KW_PSET || k == KW_PRESET ||
                    k == KW_PAINT || k == KW_OPEN || k == KW_CLOSE || k == KW_GET || k == KW_PUT ||
                    k == KW_ENTER || k == KW_RUN || k == KW_CHAIN || k == KW_LOAD || k == KW_SAVE ||
                    k == KW_KILL || k == KW_NAME || k == KW_FILES || k == KW_POKE || k == KW_OUT ||
                    k == KW_SYSTEM || k == KW_DEF || k == KW_READ || k == KW_DATA || k == KW_RESTORE ||
                    k == KW_ELSE || k == KW_THEN) {
                    break;
                }
            }
            if (tok.type == TOK_IDENT) {
                if (tok.length == 4 && (strncasecmp(tok.start, "ELSE", 4) == 0 || strncasecmp(tok.start, "THEN", 4) == 0)) {
                    break;
                }
                // Check if this identifier is followed by '=' -> start of assignment statement
                LexerContext *peek_lex = lex_init(vm_get_mem(vm), lex_get_pos(lex));
                if (peek_lex) {
                    lex_next(peek_lex);
                    BppToken pt = lex_peek(peek_lex);
                    if (pt.type == TOK_EQ) {
                        lex_shutdown(peek_lex);
                        break;
                    }
                    lex_shutdown(peek_lex);
                }
            }
        }

        // Evaluate expression
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            return err;
        }

        // Print value
        char buf[256];
        if (val.type == VAL_STRING) {
            const char *str_data_ptr = str_data(val.as.string);
            size_t len = strlen(str_data_ptr);
            vdev_puts(vdev, str_data_ptr);
            // Release temporary string returned by expression
            if (val.as.string) {
                str_release(str_ctx, val.as.string);
            }
            last_was_sep = false;
            col += len;
        } else if (val.type == VAL_COMPLEX) {
            char r_buf[128];
            char i_buf[128];
            format_double_clean(r_buf, sizeof(r_buf), val.as.complex_val.real, false, false);
            format_double_clean(i_buf, sizeof(i_buf), val.as.complex_val.imag, false, false);
            snprintf(buf, sizeof(buf), "(%s, %s) ", r_buf, i_buf);
            size_t len = strlen(buf);
            vdev_puts(vdev, buf);
            last_was_sep = false;
            col += len;
        } else {
            // Number: positive prints with leading space, negative with minus. Trailing space follows.
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



BppError stmt_display_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_using = false;
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); // Consume USING
        is_using = true;
    }

    VDevContext *vdev = vm_get_vdev(vm);
    int init_col = 0;
    VConContext *vcon = vm_get_vcon(vm);
    if (vcon) {
        vcon_get_cursor(vcon, 0, NULL, &init_col);
    }
    size_t col = (init_col >= 0) ? (size_t)init_col : 0;

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
                size_t zone_width = 15;
                size_t spaces = zone_width - (col % zone_width);
                for (size_t i = 0; i < spaces; ++i) {
                    vdev_putc(vdev, ' ');
                }
                last_was_sep = true;
                col += spaces;
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
                const char *sdata = str_data(val.as.string);
                size_t slen = strlen(sdata);
                vdev_puts(vdev, sdata);
                str_release(vm_get_str(vm), val.as.string);
                col += slen;
            } else if (val.type == VAL_COMPLEX) {
                char cbuf[128];
                char r_buf[64], i_buf[64];
                num_format_serialize(r_buf, sizeof(r_buf), val.as.complex_val.real);
                double imag = val.as.complex_val.imag;
                if (imag >= 0.0) {
                    num_format_serialize(i_buf, sizeof(i_buf), imag);
                    snprintf(cbuf, sizeof(cbuf), " %s+%sI ", r_buf, i_buf);
                } else {
                    num_format_serialize(i_buf, sizeof(i_buf), -imag);
                    snprintf(cbuf, sizeof(cbuf), " %s-%sI ", r_buf, i_buf);
                }
                vdev_puts(vdev, cbuf);
                col += strlen(cbuf);
            } else {
                char buf[64];
                num_format_serialize(buf, sizeof(buf), val.as.number);
                vdev_puts(vdev, " ");
                vdev_puts(vdev, buf);
                vdev_puts(vdev, " ");
                col += (strlen(buf) + 2);
            }
            last_was_sep = false;
        }
        if (!last_was_sep) {
            vdev_putc(vdev, '\n');
        }
    }

    return err;
}
