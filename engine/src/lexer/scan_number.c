// FILENAME: scan_number.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, float_parse.h, float_parse.c)
// NEEDS: libcore (num_parse.h, num_parse.c)
// NEEDS: libengine (lexer.h, lexer.c, lexer_internal.h)
// Implements lexical scanning and token stream processing for scan_number.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/float_parse.h"
#include "runtime/conv/num_parse.h"

//
// ---- Radix & Base Literal Scanners ----

// parses &H hex, &O/& octal, &B binary, &D decimal, and C-style 0x/0o/0b prefixes
bool scan_try_radix_number(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;

    // Vintage BASIC radix literal starting with '&'
    if (*ctx->pos == '&') {
        const char *orig = ctx->pos;
        ctx->pos++;
        long val = 0;
        bool found = false;

        if (*ctx->pos == 'H' || *ctx->pos == 'h') {
            ctx->pos++;
            while (*ctx->pos) {
                char hc = *ctx->pos;
                if (hc >= '0' && hc <= '9') {
                    val = val * 16 + (hc - '0');
                    found = true;
                } else if (hc >= 'A' && hc <= 'F') {
                    val = val * 16 + (hc - 'A' + 10);
                    found = true;
                } else if (hc >= 'a' && hc <= 'f') {
                    val = val * 16 + (hc - 'a' + 10);
                    found = true;
                } else {
                    break;
                }
                ctx->pos++;
            }
        } else if (*ctx->pos == 'O' || *ctx->pos == 'o') {
            ctx->pos++;
            while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                val = val * 8 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos == 'B' || *ctx->pos == 'b') {
            ctx->pos++;
            while (*ctx->pos == '0' || *ctx->pos == '1') {
                val = val * 2 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos == 'D' || *ctx->pos == 'd') {
            ctx->pos++;
            while (*ctx->pos >= '0' && *ctx->pos <= '9') {
                val = val * 10 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos == 'A' || *ctx->pos == 'a') {
            ctx->pos++;
            if (*ctx->pos == '"' || *ctx->pos == '\'') {
                char quote = *ctx->pos;
                ctx->pos++;
                if (*ctx->pos && *ctx->pos != quote) {
                    val = (unsigned char)*ctx->pos;
                    found = true;
                    ctx->pos++;
                }
                if (*ctx->pos == quote) {
                    ctx->pos++;
                }
            } else if (*ctx->pos) {
                val = (unsigned char)*ctx->pos;
                found = true;
                ctx->pos++;
            }
        } else if (*ctx->pos >= '0' && *ctx->pos <= '7') {
            while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                val = val * 8 + (*ctx->pos - '0');
                found = true;
                ctx->pos++;
            }
        }

        if (found) {
            tok->type = TOK_NUMBER;
            tok->length = ctx->pos - orig;
            tok->as.number = (double)val;
            return true;
        }
        ctx->pos = orig;
        return false;
    }

    // Modern C-style prefix starting with '0'
    if (*ctx->pos == '0' && *(ctx->pos + 1) != '\0') {
        char next = *(ctx->pos + 1);
        if (next == 'x' || next == 'X' || next == 'o' || next == 'O' ||
            next == 'b' || next == 'B' || next == 'd' || next == 'D') {
            const char *orig = ctx->pos;
            ctx->pos += 2;
            long val = 0;
            bool found = false;

            if (next == 'x' || next == 'X') {
                while (*ctx->pos) {
                    char hc = *ctx->pos;
                    if (hc >= '0' && hc <= '9') {
                        val = val * 16 + (hc - '0');
                        found = true;
                    } else if (hc >= 'A' && hc <= 'F') {
                        val = val * 16 + (hc - 'A' + 10);
                        found = true;
                    } else if (hc >= 'a' && hc <= 'f') {
                        val = val * 16 + (hc - 'a' + 10);
                        found = true;
                    } else {
                        break;
                    }
                    ctx->pos++;
                }
            } else if (next == 'o' || next == 'O') {
                while (*ctx->pos >= '0' && *ctx->pos <= '7') {
                    val = val * 8 + (*ctx->pos - '0');
                    found = true;
                    ctx->pos++;
                }
            } else if (next == 'd' || next == 'D') {
                while (*ctx->pos >= '0' && *ctx->pos <= '9') {
                    val = val * 10 + (*ctx->pos - '0');
                    found = true;
                    ctx->pos++;
                }
            } else {
                while (*ctx->pos == '0' || *ctx->pos == '1') {
                    val = val * 2 + (*ctx->pos - '0');
                    found = true;
                    ctx->pos++;
                }
            }

            if (found) {
                tok->type = TOK_NUMBER;
                tok->length = ctx->pos - orig;
                tok->as.number = (double)val;
                return true;
            }
            ctx->pos = orig;
            return false;
        }
    }

    return false;
}

// parses base-10 integer and floating-point numeric constants
bool scan_try_decimal_number(LexerContext *ctx, BppToken *tok) {
    if (!ctx || !ctx->pos || !tok) return false;

    if (runtime_isdigit((unsigned char)*ctx->pos) || (*ctx->pos == '.' && runtime_isdigit((unsigned char)*(ctx->pos + 1)))) {
        char *endptr;
        double val = runtime_strtod(ctx->pos, &endptr);
        if (endptr != ctx->pos) {
            if ((*endptr == 'I' || *endptr == 'i') && !runtime_isalnum((unsigned char)*(endptr + 1)) && *(endptr + 1) != '_') {
                tok->type = TOK_IMAGINARY;
                tok->length = (endptr + 1) - ctx->pos;
                tok->as.number = val;
                ctx->pos = endptr + 1;
                return true;
            }
            tok->type = TOK_NUMBER;
            tok->length = endptr - ctx->pos;
            tok->as.number = val;
            ctx->pos = endptr;
            return true;
        }
    }
    return false;
}

