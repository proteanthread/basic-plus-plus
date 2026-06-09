/*
 * =====================================================================
 * BASIC++ Interpreter - detok.h
 * =====================================================================
 *
 * Detokenizer module interface.
 *
 * PURPOSE:
 * Converts tokens back into human-readable text. This is a
 * standalone diagnostic module used for:
 *
 * 1. Debugging - display the token stream from any line.
 * 2. Future compiler support - reconstruct source from tokens.
 * 3. Validation - verify that tokenization is reversible.
 *
 * since programs are stored as plain text (not
 * tokenized), the detokenizer is primarily a debugging tool.
 * It becomes essential in future phases if tokenized storage
 * or binary program formats are introduced.
 *
 * HOW TO EXTEND:
 * As new token types are added (e.g., for new operators or
 * keywords), update detok_token_to_string() in detok.c to
 * handle them.
 *
 * =====================================================================
 */

#ifndef BASICPP_DETOK_H
#define BASICPP_DETOK_H

#include "lexer.h"

/*
 * detok_token_to_string - Convert a token to its string representation.
 *
 * Writes the human-readable form of the token into the provided
 * buffer. The buffer must be at least 'bufsize' bytes.
 *
 * Returns the number of characters written (excluding null
 * terminator), or -1 if the buffer is too small.
 *
 * Parameters:
 * tok - pointer to the token to convert
 * buf - output buffer
 * bufsize - size of the output buffer
 */
int detok_token_to_string(const Token *tok, char *buf, int bufsize);

/*
 * detok_dump_line - Tokenize a line and print all tokens.
 *
 * Diagnostic function: tokenizes the given source line and
 * prints each token's type and value to stdout. Useful for
 * debugging the lexer.
 *
 * Parameters:
 * source - null-terminated source line
 */
void detok_dump_line(const char *source);

/*
 * detok_keyword_name - Return the string name for a KeywordId.
 *
 * Returns a pointer to a static string, or NULL if the id
 * is out of range. Used by HELP to convert bare keywords
 * back to their name for topic lookup.
 */
const char *detok_keyword_name(KeywordId kw);

#endif /* BASICPP_DETOK_H */
