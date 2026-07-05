/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: format_input_core.h
 * Subsystem: Formatted String Matcher Parser
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Parses templates and extracts matching values (INPUT USING).
 *
 * 2. WHAT TO EXPECT:
 *    Matches variables according to template masks.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Character separator symbols, format pattern codes.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Parser scanning loops.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If values parse incorrectly, verify template length.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE FORMAT INPUT SUBSYSTEM
 * File: format_input_core.h
 * =====================================================================
 * Standard C interface for input format parsing and validation.
 * Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_FORMAT_INPUT_CORE_H
#define STANDALONE_FORMAT_INPUT_CORE_H

#include <stdio.h>

/* InputFormatSpec - Parsed input format specification. */
typedef struct {
    int numeric_only;      /* N: accept only digits, +, -, . */
    int hex_only;          /* H: accept only hex digits */
    int octal_only;        /* O: accept only 0-7 */
    int text_only;         /* T: reject digits */
    int force_upper;       /* U: convert to uppercase */
    int force_lower;       /* L: convert to lowercase */
    int max_length;        /* Sn: max string length */
    int has_default;       /* D: has default value */
    char default_val[256]; /* default value string */
} InputFormatSpec;

/* Protected Input Reader (backspace protection) */
int input_read_protected(char *buf, int maxlen, const char *prompt);

/* Format Parsing and Validation */
void input_parse_format(const char *fmt, int flen, InputFormatSpec *spec);
int input_validate(char *buf, int len, const InputFormatSpec *spec);

/* Main Entry Point */
int format_input_using(char *buf, int maxlen, const char *fmt, int flen, const char *prompt);

#endif /* STANDALONE_FORMAT_INPUT_CORE_H */
