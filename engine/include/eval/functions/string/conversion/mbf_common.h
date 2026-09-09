// FILENAME: mbf_common.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cvsmbf.c, cvdmbf.c, mksmbf.c, mkdmbf.c)
// NEEDS: libcore (math.h, memory.h)
// Provides shared bit manipulation algorithms for Microsoft Binary Format (MBF).

#ifndef MBF_COMMON_H
#define MBF_COMMON_H

#include "runtime/math.h"
#include "runtime/memory.h"
#include <stdint.h>
#include "runtime/string/memops.h"
#include "runtime/math/math.h"

static inline double mbf4_to_double(const uint8_t *b) {
    uint8_t runtime_exp = b[3];
    if (runtime_exp == 0) return 0.0;
    
    double mant = ((double)((b[2] & 0x7F) | 0x80) * 65536.0 + (double)b[1] * 256.0 + (double)b[0]) / 16777216.0;
    double val = runtime_ldexp(mant, (int)runtime_exp - 128);
    if (b[2] & 0x80) val = -val;
    return val;
}

static inline double mbf8_to_double(const uint8_t *b) {
    uint8_t runtime_exp = b[7];
    if (runtime_exp == 0) return 0.0;
    
    double hi = ((double)((b[6] & 0x7F) | 0x80) * 256.0 + (double)b[5]) * 65536.0 + ((double)b[4] * 256.0 + (double)b[3]);
    double lo = ((double)b[2] * 256.0 + (double)b[1]) * 256.0 + (double)b[0];
    double mant = (hi * 16777216.0 + lo) / 72057594037927936.0;
    
    double val = runtime_ldexp(mant, (int)runtime_exp - 128);
    if (b[6] & 0x80) val = -val;
    return val;
}

static inline void double_to_mbf4(double val, uint8_t *b) {
    runtime_memset(b, 0, 4);
    if (val == 0.0) return;
    
    int runtime_exp = 0;
    double mant = runtime_frexp(runtime_fabs(val), &runtime_exp);
    int mbf_exp = runtime_exp + 128;
    if (mbf_exp <= 0) return; // Underflow
    if (mbf_exp > 255) mbf_exp = 255; // Overflow clamp
    
    uint32_t m_int = (uint32_t)(mant * 16777216.0);
    b[3] = (uint8_t)mbf_exp;
    b[2] = (uint8_t)((m_int >> 16) & 0x7F) | (val < 0.0 ? 0x80 : 0);
    b[1] = (uint8_t)((m_int >> 8) & 0xFF);
    b[0] = (uint8_t)(m_int & 0xFF);
}

static inline void double_to_mbf8(double val, uint8_t *b) {
    runtime_memset(b, 0, 8);
    if (val == 0.0) return;
    
    int runtime_exp = 0;
    double mant = runtime_frexp(runtime_fabs(val), &runtime_exp);
    int mbf_exp = runtime_exp + 128;
    if (mbf_exp <= 0) return;
    if (mbf_exp > 255) mbf_exp = 255;
    
    uint64_t m_int = (uint64_t)(mant * 72057594037927936.0);

    b[7] = (uint8_t)mbf_exp;
    b[6] = (uint8_t)((m_int >> 48) & 0x7F) | (val < 0.0 ? 0x80 : 0);
    b[5] = (uint8_t)((m_int >> 40) & 0xFF);
    b[4] = (uint8_t)((m_int >> 32) & 0xFF);
    b[3] = (uint8_t)((m_int >> 24) & 0xFF);
    b[2] = (uint8_t)((m_int >> 16) & 0xFF);
    b[1] = (uint8_t)((m_int >> 8) & 0xFF);
    b[0] = (uint8_t)(m_int & 0xFF);
}

#endif // MBF_COMMON_H
