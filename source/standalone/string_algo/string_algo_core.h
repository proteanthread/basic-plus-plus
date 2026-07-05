/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: string_algo_core.h
 * Subsystem: String Utilities and Radix Converter
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Converts integers to binary/octal/hex and finds index.
 *
 * 2. WHAT TO EXPECT:
 *    Returns string representations of numeric values.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Radix converter boundaries.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Character mapping tables.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If conversion overflows, verify input scale.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE STRING ALGORITHMS CORE
 * File: string_algo_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_STRING_ALGO_CORE_H
#define BASICPP_STANDALONE_STRING_ALGO_CORE_H

int string_algo_core_hex(unsigned long val, char *dest, int max_len);
int string_algo_core_oct(unsigned long val, char *dest, int max_len);
int string_algo_core_bin(unsigned long val, char *dest, int max_len);
int string_algo_core_instr(const char *haystack, int hl, const char *needle, int nl, int start_off);

#endif // BASICPP_STANDALONE_STRING_ALGO_CORE_H
