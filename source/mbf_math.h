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
 *    - Interface definitions for MBF-to-IEEE floating-point conversion routines.
 *    - 32-bit and 64-bit Microsoft Binary Format conversion interfaces.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GW_MATH_MBF_H
#define GW_MATH_MBF_H

#include <stdint.h>

// Convert 4-byte Single Precision MBF to double
double gw_mbf32_to_double(const uint8_t *mbf);

// Convert double to 4-byte Single Precision MBF
void gw_double_to_mbf32(double val, uint8_t *mbf);

// Convert 8-byte Double Precision MBF to double
double gw_mbf64_to_double(const uint8_t *mbf);

// Convert double to 8-byte Double Precision MBF
void gw_double_to_mbf64(double val, uint8_t *mbf);

#endif // GW_MATH_MBF_H
