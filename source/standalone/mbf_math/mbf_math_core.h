/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mbf_math_core.h
 * Subsystem: Microsoft Binary Format Float Converter
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Translates Microsoft Binary Format (MBF) float/double to standard IEEE formats.
 *
 * 2. WHAT TO EXPECT:
 *    Precision bit operations converting byte floats.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Precision limits.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    MBF layout format.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If floats convert to trash, check CPU endianness.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MICROSOFT BINARY FORMAT (MBF) MATH SUBSYSTEM
 * File: mbf_math_core.h
 * =====================================================================
 * Standard C interface for converting 32-bit and 64-bit MBF float values
 * to and from standard IEEE doubles. Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_GW_MATH_MBF_CORE_H
#define STANDALONE_GW_MATH_MBF_CORE_H

#include <stdint.h>

/* Convert 4-byte Single Precision MBF to double */
double gw_mbf32_to_double(const uint8_t *mbf);

/* Convert double to 4-byte Single Precision MBF */
void gw_double_to_mbf32(double val, uint8_t *mbf);

/* Convert 8-byte Double Precision MBF to double */
double gw_mbf64_to_double(const uint8_t *mbf);

/* Convert double to 8-byte Double Precision MBF */
void gw_double_to_mbf64(double val, uint8_t *mbf);

#endif /* STANDALONE_GW_MATH_MBF_CORE_H */
