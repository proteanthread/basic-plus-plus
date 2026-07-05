/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mbf_math_core.c
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
 * File: mbf_math_core.c
 * =====================================================================
 * Standard C implementation for converting 32-bit and 64-bit MBF float values
 * to and from standard IEEE doubles. Completely independent of the interpreter.
 * ===================================================================== */

#include "mbf_math_core.h"
#include <math.h>
#include <string.h>

double gw_mbf32_to_double(const uint8_t *mbf) {
    uint8_t exponent = mbf[3];
    if (exponent == 0) {
        return 0.0;
    }
    
    uint8_t sign = mbf[2] >> 7;
    uint32_t mantissa = ((mbf[2] & 0x7F) << 16) | (mbf[1] << 8) | mbf[0];
    
    double val = (1.0 + (double)mantissa / 8388608.0) * ldexp(1.0, (int)exponent - 129);
    return sign ? -val : val;
}

void gw_double_to_mbf32(double val, uint8_t *mbf) {
    memset(mbf, 0, 4);
    if (val == 0.0) {
        return;
    }
    
    uint8_t sign = 0;
    if (val < 0.0) {
        sign = 1;
        val = -val;
    }
    
    int exp_int;
    double mant = frexp(val, &exp_int);
    
    mant *= 2.0;
    exp_int -= 1;
    
    int biased_exp = exp_int + 129;
    if (biased_exp <= 0) {
        return;
    }
    if (biased_exp > 255) {
        mbf[3] = 255;
        mbf[2] = (sign << 7) | 0x7F;
        mbf[1] = 0xFF;
        mbf[0] = 0xFF;
        return;
    }
    
    double fraction = mant - 1.0;
    uint32_t mant_int = (uint32_t)(fraction * 8388608.0 + 0.5);
    
    if (mant_int >= 8388608) {
        mant_int = 0;
        biased_exp++;
        if (biased_exp > 255) {
            mbf[3] = 255;
            mbf[2] = (sign << 7) | 0x7F;
            mbf[1] = 0xFF;
            mbf[0] = 0xFF;
            return;
        }
    }
    
    mbf[3] = (uint8_t)biased_exp;
    mbf[2] = (sign << 7) | ((mant_int >> 16) & 0x7F);
    mbf[1] = (mant_int >> 8) & 0xFF;
    mbf[0] = mant_int & 0xFF;
}

double gw_mbf64_to_double(const uint8_t *mbf) {
    uint8_t exponent = mbf[7];
    if (exponent == 0) {
        return 0.0;
    }
    
    uint8_t sign = mbf[6] >> 7;
    uint64_t mantissa = ((uint64_t)(mbf[6] & 0x7F) << 48) |
                        ((uint64_t)mbf[5] << 40) |
                        ((uint64_t)mbf[4] << 32) |
                        ((uint64_t)mbf[3] << 24) |
                        ((uint64_t)mbf[2] << 16) |
                        ((uint64_t)mbf[1] << 8) |
                        mbf[0];
    
    double val = (1.0 + (double)mantissa / 562949953421312.0) * ldexp(1.0, (int)exponent - 129);
    return sign ? -val : val;
}

void gw_double_to_mbf64(double val, uint8_t *mbf) {
    memset(mbf, 0, 8);
    if (val == 0.0) {
        return;
    }
    
    uint8_t sign = 0;
    if (val < 0.0) {
        sign = 1;
        val = -val;
    }
    
    int exp_int;
    double mant = frexp(val, &exp_int);
    
    mant *= 2.0;
    exp_int -= 1;
    
    int biased_exp = exp_int + 129;
    if (biased_exp <= 0) {
        return;
    }
    if (biased_exp > 255) {
        mbf[7] = 255;
        mbf[6] = (sign << 7) | 0x7F;
        memset(mbf, 0xFF, 6);
        return;
    }
    
    double fraction = mant - 1.0;
    uint64_t mant_int = (uint64_t)(fraction * 562949953421312.0 + 0.5);
    
    if (mant_int >= 562949953421312ULL) {
        mant_int = 0;
        biased_exp++;
        if (biased_exp > 255) {
            mbf[7] = 255;
            mbf[6] = (sign << 7) | 0x7F;
            memset(mbf, 0xFF, 6);
            return;
        }
    }
    
    mbf[7] = (uint8_t)biased_exp;
    mbf[6] = (sign << 7) | ((mant_int >> 48) & 0x7F);
    mbf[5] = (mant_int >> 40) & 0xFF;
    mbf[4] = (mant_int >> 32) & 0xFF;
    mbf[3] = (mant_int >> 24) & 0xFF;
    mbf[2] = (mant_int >> 16) & 0xFF;
    mbf[1] = (mant_int >> 8) & 0xFF;
    mbf[0] = mant_int & 0xFF;
}
