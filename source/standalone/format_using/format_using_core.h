/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: format_using_core.h
 * Subsystem: Formatted PRINT USING Formatter
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Formats numbers and strings based on templates (PRINT USING).
 *
 * 2. WHAT TO EXPECT:
 *    Renders padded values and adds decimal signs.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Currency signs, digit punctuation delimiters.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Scientific format formatting math.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If text overflows, verify formatting mask width.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE FORMAT USING SUBSYSTEM
 * File: format_using_core.h
 * =====================================================================
 * Standard C interface for formatting numeric and string values using
 * classic BASIC specifiers. Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_FORMAT_USING_CORE_H
#define STANDALONE_FORMAT_USING_CORE_H

#include <stdio.h>

/* Character & String Output Utilities */
void format_using_emit_char(FILE *fp, char ch);
void format_using_emit_str(FILE *fp, const char *s, int len);
void format_using_emit_fill(FILE *fp, char ch, int n);

/* Escape Sequence Resolver */
int format_using_escape(FILE *fp, const char *fmt, int pos, int flen);

/* Formatter Core Functions */
int format_using_numeric(FILE *fp, const char *fmt, int flen, int *pos, double value);
int format_using_radix(FILE *fp, const char *fmt, int flen, int *pos, double value, int rep);
int format_using_string_field(FILE *fp, const char *fmt, int flen, int *pos, const char *str, int slen);

#endif /* STANDALONE_FORMAT_USING_CORE_H */
