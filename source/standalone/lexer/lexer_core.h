/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: lexer_core.h
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
 * File: lexer_core.h
 * ===================================================================== */

#ifndef STANDALONE_LEXER_CORE_H
#define STANDALONE_LEXER_CORE_H

typedef enum {
    LEX_TOK_EOF = 0,
    LEX_TOK_NUMBER,
    LEX_TOK_FLOAT_LIT,
    LEX_TOK_IMAGINARY,
    LEX_TOK_STRING,
    LEX_TOK_IDENTIFIER,
    LEX_TOK_OPERATOR,
    LEX_TOK_PUNCTUATION,
    LEX_TOK_ERROR
} LexCoreTokenType;

typedef struct {
    LexCoreTokenType type;
    const char *text;
    int length;
    double number_value;
    long integer_value;
    int pos;
} LexCoreToken;

/* Scans the next raw token from the source line starting at *pos.
 * Updates *pos to the index after the token. */
LexCoreToken lexer_core_next_token(const char *source, int *pos, int length);

#endif /* STANDALONE_LEXER_CORE_H */
