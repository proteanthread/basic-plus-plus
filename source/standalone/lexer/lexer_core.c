/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lexer_core.c
 * Subsystem: Parser Independent Tokenizer Scanner
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Converts source text to token streams (zero-copy).
 *
 * 2. WHAT TO EXPECT:
 *    Fast, memory-safe lexical analysis.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Valid syntax characters, separators.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Token scanner state transitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If parsing corrupts, check source character set encoding.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE TOKENIZER ENGINE
 * File: lexer_core.c
 * ===================================================================== */

#include "lexer_core.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static int is_alpha_core(char c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '$');
}

static int is_digit_core(char c)
{
    return (c >= '0' && c <= '9');
}

static int is_hex_digit(char c)
{
    return (is_digit_core(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

static int is_oct_digit(char c)
{
    return (c >= '0' && c <= '7');
}

static int is_bin_digit(char c)
{
    return (c == '0' || c == '1');
}

static int is_ident_char(char c)
{
    return (is_alpha_core(c) || is_digit_core(c));
}

static int is_type_suffix(char c)
{
    return (c == '$' || c == '%' || c == '&' || c == '!' || c == '#' || c == '~');
}

static void skip_whitespace_core(const char *source, int *pos, int length)
{
    while (*pos < length) {
        char c = source[*pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            (*pos)++;
        } else {
            break;
        }
    }
}

LexCoreToken lexer_core_next_token(const char *source, int *pos, int length)
{
    LexCoreToken tok;
    memset(&tok, 0, sizeof(tok));

    skip_whitespace_core(source, pos, length);
    tok.pos = *pos;

    if (*pos >= length) {
        tok.type = LEX_TOK_EOF;
        tok.text = &source[*pos];
        tok.length = 0;
        return tok;
    }

    char c = source[*pos];

    /* Hex, octal, binary literals: &H, &O, &B */
    if (c == '&' && *pos + 1 < length) {
        char next = source[*pos + 1];
        if (next == 'H' || next == 'h' || next == 'O' || next == 'o' || next == 'B' || next == 'b') {
            int start = *pos;
            *pos += 2;
            long val = 0;
            if (next == 'H' || next == 'h') {
                while (*pos < length && is_hex_digit(source[*pos])) {
                    char d = source[*pos];
                    val *= 16;
                    if (is_digit_core(d)) val += (d - '0');
                    else if (d >= 'A' && d <= 'F') val += (10 + d - 'A');
                    else val += (10 + d - 'a');
                    (*pos)++;
                }
            } else if (next == 'O' || next == 'o') {
                while (*pos < length && is_oct_digit(source[*pos])) {
                    val = val * 8 + (source[*pos] - '0');
                    (*pos)++;
                }
            } else {
                while (*pos < length && is_bin_digit(source[*pos])) {
                    val = val * 2 + (source[*pos] - '0');
                    (*pos)++;
                }
            }
            tok.type = LEX_TOK_NUMBER;
            tok.text = &source[start];
            tok.length = *pos - start;
            tok.integer_value = val;
            tok.number_value = (double)val;
            return tok;
        }
    }

    /* String literal */
    if (c == '"') {
        int start = *pos;
        (*pos)++; /* skip open quote */
        tok.text = &source[*pos];
        while (*pos < length && source[*pos] != '"' && source[*pos] != '\n') {
            (*pos)++;
        }
        tok.length = *pos - (start + 1);
        if (*pos < length && source[*pos] == '"') {
            (*pos)++; /* skip close quote */
        }
        tok.type = LEX_TOK_STRING;
        return tok;
    }

    /* Operators and Punctuation */
    /* Check multi-char operators first */
    if (*pos + 1 < length) {
        char c2 = source[*pos + 1];
        if ((c == '<' && c2 == '=') || (c == '>' && c2 == '=') || (c == '<' && c2 == '>') ||
            (c == ':' && c2 == ':') || (c == '>' && c2 == '>')) {
            tok.type = LEX_TOK_OPERATOR;
            tok.text = &source[*pos];
            tok.length = 2;
            *pos += 2;
            return tok;
        }
    }

    /* Single character operators / punctuation */
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '^' || c == '\\' || c == '|' || c == '&') {
        tok.type = LEX_TOK_OPERATOR;
        tok.text = &source[*pos];
        tok.length = 1;
        (*pos)++;
        return tok;
    }

    if (c == '(' || c == ')' || c == ',' || c == ';' || c == ':' || c == '#' || c == '@' || c == '.') {
        tok.type = LEX_TOK_PUNCTUATION;
        tok.text = &source[*pos];
        tok.length = 1;
        (*pos)++;
        return tok;
    }

    /* Numeric literal (integer or float or imaginary) */
    if (is_digit_core(c) || (c == '.' && *pos + 1 < length && is_digit_core(source[*pos + 1]))) {
        int start = *pos;
        int is_float = 0;
        if (c == '.') {
            is_float = 1;
            (*pos)++;
        }
        while (*pos < length && is_digit_core(source[*pos])) {
            (*pos)++;
        }
        if (!is_float && *pos < length && source[*pos] == '.') {
            /* Try to check if next char is digit or if it is just a dot */
            if (*pos + 1 >= length || !isalpha((unsigned char)source[*pos + 1])) {
                is_float = 1;
                (*pos)++;
                while (*pos < length && is_digit_core(source[*pos])) {
                    (*pos)++;
                }
            }
        }
        /* Exponent */
        if (*pos < length && (source[*pos] == 'E' || source[*pos] == 'e' || source[*pos] == 'D' || source[*pos] == 'd')) {
            is_float = 1;
            (*pos)++;
            if (*pos < length && (source[*pos] == '+' || source[*pos] == '-')) {
                (*pos)++;
            }
            while (*pos < length && is_digit_core(source[*pos])) {
                (*pos)++;
            }
        }
        /* Imaginary suffix 'i' */
        int is_imag = 0;
        if (*pos < length && source[*pos] == 'i') {
            is_imag = 1;
            (*pos)++;
        }

        tok.text = &source[start];
        tok.length = *pos - start;
        char temp[64];
        if (tok.length < (int)sizeof(temp)) {
            memcpy(temp, tok.text, (size_t)tok.length);
            temp[tok.length] = '\0';
            if (is_imag) temp[tok.length - 1] = '\0'; /* strip 'i' */
            if (is_float) {
                /* replace 'D'/'d' with 'e' for strtod */
                for (int idx = 0; idx < tok.length; idx++) {
                    if (temp[idx] == 'D' || temp[idx] == 'd') temp[idx] = 'e';
                }
                tok.number_value = strtod(temp, NULL);
                tok.type = LEX_TOK_FLOAT_LIT;
            } else {
                tok.integer_value = strtol(temp, NULL, 10);
                tok.number_value = (double)tok.integer_value;
                tok.type = LEX_TOK_NUMBER;
            }
        } else {
            tok.type = LEX_TOK_ERROR;
        }
        if (is_imag) {
            tok.type = LEX_TOK_IMAGINARY;
        }
        return tok;
    }

    /* Identifier or Keyword */
    if (is_alpha_core(c) || c == '.') {
        int start = *pos;
        (*pos)++;
        while (*pos < length && is_ident_char(source[*pos])) {
            (*pos)++;
        }
        /* check optional type suffix */
        if (*pos < length && is_type_suffix(source[*pos])) {
            (*pos)++;
        }
        tok.type = LEX_TOK_IDENTIFIER;
        tok.text = &source[start];
        tok.length = *pos - start;
        return tok;
    }

    /* Single unknown/error char */
    tok.type = LEX_TOK_ERROR;
    tok.text = &source[*pos];
    tok.length = 1;
    (*pos)++;
    return tok;
}
