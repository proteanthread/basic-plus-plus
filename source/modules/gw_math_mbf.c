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
 *    - Microsoft Binary Format (MBF) single/double float conversion and operations.
 *    - Raw bit manipulation to reconstruct double-precision values from classic MBF format.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "gw_math_mbf.h"
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
    
    // frexp returns mantissa in [0.5, 1.0)
    // MBF expects exponent biased by 128 for normalized mantissa in [1.0, 2.0)
    // So mant = mant * 2, exp_int = exp_int - 1
    mant *= 2.0;
    exp_int -= 1;
    
    int biased_exp = exp_int + 129; // 128 + 1
    if (biased_exp <= 0) {
        // Underflow
        return;
    }
    if (biased_exp > 255) {
        // Overflow - clamp to maximum MBF Single value
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
        return; // Underflow
    }
    if (biased_exp > 255) {
        // Overflow
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
