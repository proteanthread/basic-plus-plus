// FILENAME: print_file.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, print_file.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PRINT_FILE statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/print_file.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/num_format.h"
#include "platform/platform.h"
#include <string.h>
#include <stdio.h>

void stmt_print_file_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PRINT#",
        .category = "Filesystem I/O",
        .syntax = "PRINT #file_num, expression_list",
        .help_text = "Writes sequential formatted text data to an open disk file channel.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 54: Bad File Mode"
    };
    microlib_register(&meta);
}

BppError stmt_print_file_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int channel = (int)ch_val.as.number;
    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA || tok.type == TOK_SEMICOLON) {
        lex_next(lex);
    }

    bool trailing_sep = false;
    size_t col = 0;

    char line_buf[4096];
    size_t buf_len = 0;

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH ||
            (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
            break;
        }

        // Check if next token begins a space-separated statement or block
        if (!trailing_sep) {
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
                    k == KW_SYSTEM || k == KW_DEF || k == KW_READ || k == KW_DATA || k == KW_RESTORE) {
                    break;
                }
            }
            if (tok.type == TOK_IDENT) {
                const char *p = tok.start + tok.length;
                while (*p && (*p == ' ' || *p == '\t')) p++;
                if (*p == '=') {
                    break;
                }
            }
        }

        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            size_t zone_width = vm ? (size_t)vm_get_zone_width(vm) : 14;
            if (zone_width == 0) zone_width = 14;
            size_t spaces = zone_width - (col % zone_width);
            for (size_t i = 0; i < spaces; ++i) {
                if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = ' ';
            }
            col += spaces;
            trailing_sep = true;
            continue;
        }

        if (tok.type == TOK_SEMICOLON) {
            lex_next(lex);
            trailing_sep = true;
            continue;
        }

        // Check for TAB, SPC, SPA, or LIN print modifiers
        if (tok.type == TOK_IDENT && tok.length == 3 &&
            (platform_strncasecmp(tok.start, "TAB", 3) == 0 ||
             platform_strncasecmp(tok.start, "SPC", 3) == 0 ||
             platform_strncasecmp(tok.start, "SPA", 3) == 0 ||
             platform_strncasecmp(tok.start, "LIN", 3) == 0)) {
            bool is_tab = (platform_strncasecmp(tok.start, "TAB", 3) == 0);
            bool is_lin = (platform_strncasecmp(tok.start, "LIN", 3) == 0);
            lex_next(lex); // Consume modifier

            BppToken open_paren = lex_peek(lex);
            if (open_paren.type != TOK_LPAREN) {
                err.code = 2;
                err.message = "Expected '(' after print modifier";
                return err;
            }
            lex_next(lex); // Consume '('

            BValue arg_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            BppToken close_paren = lex_peek(lex);
            if (close_paren.type != TOK_RPAREN) {
                err.code = 2;
                err.message = "Expected ')' after print modifier argument";
                return err;
            }
            lex_next(lex); // Consume ')'

            int n = (int)arg_val.as.number;
            if (is_lin) {
                if (n == 0) {
                    if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = '\r';
                    col = 0;
                } else if (n == -1) {
                    if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = '\f';
                    col = 0;
                } else if (n > 0) {
                    for (int i = 0; i < n; ++i) {
                        if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = '\n';
                    }
                    col = 0;
                }
            } else if (is_tab) {
                int spaces = n - (int)col;
                if (spaces > 0) {
                    for (int i = 0; i < spaces; ++i) {
                        if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = ' ';
                    }
                    col += (size_t)spaces;
                }
            } else {
                // SPC or SPA
                if (n > 0) {
                    for (int i = 0; i < n; ++i) {
                        if (buf_len < sizeof(line_buf) - 1) line_buf[buf_len++] = ' ';
                    }
                    col += (size_t)n;
                }
            }
            trailing_sep = false;
            continue;
        }

        // Evaluate expression
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            char num_buf[64];
            num_format_display(num_buf, sizeof(num_buf), val.as.number, true, true);
            size_t nlen = strlen(num_buf);
            if (buf_len + nlen < sizeof(line_buf) - 1) {
                memcpy(line_buf + buf_len, num_buf, nlen);
                buf_len += nlen;
            } else {
                line_buf[buf_len] = '\0';
                file_puts(fc, channel, line_buf);
                buf_len = 0;
                file_puts(fc, channel, num_buf);
            }
            col += nlen;
        } else if (val.type == VAL_COMPLEX) {
            char cpx_buf[128];
            snprintf(cpx_buf, sizeof(cpx_buf), "(%g, %g) ", val.as.complex_val.real, val.as.complex_val.imag);
            size_t clen = strlen(cpx_buf);
            if (buf_len + clen < sizeof(line_buf) - 1) {
                memcpy(line_buf + buf_len, cpx_buf, clen);
                buf_len += clen;
            } else {
                line_buf[buf_len] = '\0';
                file_puts(fc, channel, line_buf);
                buf_len = 0;
                file_puts(fc, channel, cpx_buf);
            }
            col += clen;
        } else if (val.type == VAL_STRING && val.as.string) {
            const char *sdata = str_data(val.as.string);
            size_t slen = str_len(val.as.string);
            if (sdata && slen > 0) {
                if (buf_len + slen < sizeof(line_buf) - 1) {
                    memcpy(line_buf + buf_len, sdata, slen);
                    buf_len += slen;
                } else {
                    line_buf[buf_len] = '\0';
                    file_puts(fc, channel, line_buf);
                    buf_len = 0;
                    file_puts(fc, channel, sdata);
                }
            }
            col += slen;
            str_release(vm_get_str(vm), val.as.string);
        }

        trailing_sep = false;
    }

    if (!trailing_sep) {
        if (buf_len < sizeof(line_buf) - 1) {
            line_buf[buf_len++] = '\n';
        } else {
            line_buf[buf_len] = '\0';
            file_puts(fc, channel, line_buf);
            buf_len = 0;
            file_puts(fc, channel, "\n");
        }
    }

    if (buf_len > 0) {
        line_buf[buf_len] = '\0';
        file_puts(fc, channel, line_buf);
    }

    return err;
}

BppError stmt_file_print_handler(VMContext *vm, LexerContext *lex) {
    return stmt_print_file_handler(vm, lex);
}
