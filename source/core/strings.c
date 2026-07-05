/**
 * GW-BASIC / BASIC++ Core Engine (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Logic inside statement handlers to optimize behaviors.
 *    - Diagnostic logs and specific error message phrasing.
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables and execution loop structure.
 *    - Memory pool layouts.
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Core interpreter execution honoring C17 standards.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * -----------------------------------------------------------------------------
 */
#include "strings.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GW_String gw_str_create(const char *str, size_t len) {
    GW_String s;
    s.len = 0;
    s.ptr = NULL;
    
    if (len > 255) len = 255;
    if (len == 0) return s;
    
    s.ptr = (char *)malloc(len + 1);
    if (s.ptr) {
        if (str) {
            memcpy(s.ptr, str, len);
        }
        s.ptr[len] = '\0';
        s.len = (uint8_t)len;
    }
    return s;
}

void gw_str_free(GW_String *s) {
    if (s && s->ptr) {
        free(s->ptr);
        s->ptr = NULL;
        s->len = 0;
    }
}

GW_String gw_str_concat(const GW_String *s1, const GW_String *s2) {
    int total_len = (s1 ? s1->len : 0) + (s2 ? s2->len : 0);
    if (total_len > 255) total_len = 255;
    
    GW_String s = gw_str_create(NULL, total_len);
    if (s.ptr) {
        int offset = 0;
        if (s1 && s1->ptr && s1->len > 0) {
            int len1 = s1->len;
            if (len1 > total_len) len1 = total_len;
            memcpy(s.ptr, s1->ptr, len1);
            offset = len1;
        }
        if (s2 && s2->ptr && s2->len > 0 && offset < total_len) {
            int len2 = s2->len;
            if (offset + len2 > total_len) len2 = total_len - offset;
            memcpy(s.ptr + offset, s2->ptr, len2);
        }
    }
    return s;
}

GW_String gw_str_left(const GW_String *s, int len) {
    if (!s || len <= 0) return gw_str_create(NULL, 0);
    if (len > s->len) len = s->len;
    return gw_str_create(s->ptr, len);
}

GW_String gw_str_right(const GW_String *s, int len) {
    if (!s || len <= 0) return gw_str_create(NULL, 0);
    if (len > s->len) len = s->len;
    return gw_str_create(s->ptr + (s->len - len), len);
}

GW_String gw_str_mid(const GW_String *s, int start, int len) {
    if (!s || start <= 0 || start > s->len || len <= 0) return gw_str_create(NULL, 0);
    
    int actual_start = start - 1; // 1-indexed to 0-indexed
    int avail_len = s->len - actual_start;
    if (len > avail_len) len = avail_len;
    
    return gw_str_create(s->ptr + actual_start, len);
}

double gw_str_val(const GW_String *s) {
    if (!s || !s->ptr || s->len == 0) return 0.0;
    
    // Copy to temporary null-terminated buffer to prevent overflow read
    char temp[256];
    int len = s->len;
    if (len > 255) len = 255;
    memcpy(temp, s->ptr, len);
    temp[len] = '\0';
    
    return strtod(temp, NULL);
}

GW_String gw_str_chr(int code) {
    char c = (char)code;
    return gw_str_create(&c, 1);
}

GW_String gw_str_space(int count) {
    if (count <= 0) return gw_str_create(NULL, 0);
    if (count > 255) count = 255;
    
    char temp[256];
    memset(temp, ' ', count);
    return gw_str_create(temp, count);
}

GW_String gw_str_string(int count, const GW_String *char_s) {
    if (count <= 0 || !char_s || char_s->len == 0) return gw_str_create(NULL, 0);
    if (count > 255) count = 255;
    
    char fill_char = char_s->ptr[0];
    char temp[256];
    memset(temp, fill_char, count);
    return gw_str_create(temp, count);
}
