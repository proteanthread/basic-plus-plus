/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions for terminal input/output, line-editing, and character read utilities.
 *    - Declarations of console hooks, terminal cursor mapping, and keyboard buffers.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "legacy_compat.h"
#include <stdio.h>
#include <stdarg.h>

// Output character
void gw_console_write_char(char c);

// Get character (non-blocking, returns 0 if none)
char gw_console_read_char(void);

// Line editor
char *gw_console_read_line(char *buf, size_t max_len);

int gw_printf(const char *format, ...);
int gw_vprintf(const char *format, va_list args);
int gw_fflush(FILE *stream);

#ifndef CONSOLE_C_INTERNAL
#define printf gw_printf
#define vprintf gw_vprintf
#define fflush gw_fflush
static inline int gw_putchar(int c) {
    gw_console_write_char((char)c);
    return c;
}
#define putchar(c) gw_putchar(c)
#endif

#endif // CONSOLE_H
