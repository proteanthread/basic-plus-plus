// FILENAME: interop_marshal.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h)
// NEEDS: libcore (interop_marshal.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_marshal within BASIC++.
//
// ---- Includes ----

#include "interop/interop_marshal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* interop_strdup_safe(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    char* copy = (char*)calloc(len + 1, sizeof(char));
    if (copy != NULL) {
        memcpy(copy, str, len);
    }
    return copy;
}

InteropValue interop_value_from_number(double val) {
    InteropValue v = { .type = INTEROP_TYPE_NUMBER };
    v.as.number_val = val;
    return v;
}

InteropValue interop_value_from_string(const char* val) {
    InteropValue v = { .type = INTEROP_TYPE_STRING };
    v.as.string_val = interop_strdup_safe(val);
    if (v.as.string_val == NULL) {
        v.type = INTEROP_TYPE_NULL;
    }
    return v;
}

InteropValue interop_value_from_integer(int64_t val) {
    InteropValue v = { .type = INTEROP_TYPE_INTEGER };
    v.as.integer_val = val;
    return v;
}

InteropValue interop_value_from_boolean(bool val) {
    InteropValue v = { .type = INTEROP_TYPE_BOOLEAN };
    v.as.boolean_val = val;
    return v;
}

int interop_value_to_number(const InteropValue* val, double* out_val) {
    if (val == NULL || out_val == NULL) return -1;
    
    switch (val->type) {
        case INTEROP_TYPE_NUMBER:
            *out_val = val->as.number_val;
            return 0;
        case INTEROP_TYPE_INTEGER:
            *out_val = (double)val->as.integer_val;
            return 0;
        case INTEROP_TYPE_BOOLEAN:
            *out_val = val->as.boolean_val ? 1.0 : 0.0;
            return 0;
        case INTEROP_TYPE_STRING:
            if (val->as.string_val != NULL) {
                *out_val = strtod(val->as.string_val, NULL);
                return 0;
            }
            return -1;
        default:
            return -1;
    }
}

int interop_value_to_string(const InteropValue* val, char** out_val) {
    if (val == NULL || out_val == NULL) return -1;
    
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    
    switch (val->type) {
        case INTEROP_TYPE_STRING:
            *out_val = interop_strdup_safe(val->as.string_val);
            return (*out_val != NULL) ? 0 : -1;
        case INTEROP_TYPE_NUMBER:
            snprintf(buffer, sizeof(buffer), "%.14g", val->as.number_val);
            *out_val = interop_strdup_safe(buffer);
            return (*out_val != NULL) ? 0 : -1;
        case INTEROP_TYPE_INTEGER:
            snprintf(buffer, sizeof(buffer), "%lld", (long long)val->as.integer_val);
            *out_val = interop_strdup_safe(buffer);
            return (*out_val != NULL) ? 0 : -1;
        case INTEROP_TYPE_BOOLEAN:
            *out_val = interop_strdup_safe(val->as.boolean_val ? "true" : "false");
            return (*out_val != NULL) ? 0 : -1;
        default:
            return -1;
    }
}

int interop_value_to_integer(const InteropValue* val, int64_t* out_val) {
    if (val == NULL || out_val == NULL) return -1;
    
    switch (val->type) {
        case INTEROP_TYPE_INTEGER:
            *out_val = val->as.integer_val;
            return 0;
        case INTEROP_TYPE_NUMBER:
            *out_val = (int64_t)val->as.number_val;
            return 0;
        case INTEROP_TYPE_BOOLEAN:
            *out_val = val->as.boolean_val ? 1 : 0;
            return 0;
        case INTEROP_TYPE_STRING:
            if (val->as.string_val != NULL) {
                *out_val = (int64_t)strtoll(val->as.string_val, NULL, 10);
                return 0;
            }
            return -1;
        default:
            return -1;
    }
}

int interop_value_to_boolean(const InteropValue* val, bool* out_val) {
    if (val == NULL || out_val == NULL) return -1;
    
    switch (val->type) {
        case INTEROP_TYPE_BOOLEAN:
            *out_val = val->as.boolean_val;
            return 0;
        case INTEROP_TYPE_INTEGER:
            *out_val = (val->as.integer_val != 0);
            return 0;
        case INTEROP_TYPE_NUMBER:
            *out_val = (val->as.number_val != 0.0);
            return 0;
        case INTEROP_TYPE_STRING:
            if (val->as.string_val != NULL) {
                *out_val = (val->as.string_val[0] != '\0');
                return 0;
            }
            return -1;
        default:
            return -1;
    }
}

void interop_value_release(InteropValue* val) {
    if (val == NULL) return;
    
    if (val->type == INTEROP_TYPE_STRING && val->as.string_val != NULL) {
        free(val->as.string_val);
    }
    
    memset(val, 0, sizeof(InteropValue));
    val->type = INTEROP_TYPE_NULL;
}

int interop_value_clone(const InteropValue* src, InteropValue* dst) {
    if (src == NULL || dst == NULL) return -1;
    
    memset(dst, 0, sizeof(InteropValue));
    dst->type = src->type;
    
    switch (src->type) {
        case INTEROP_TYPE_STRING:
            dst->as.string_val = interop_strdup_safe(src->as.string_val);
            if (dst->as.string_val == NULL && src->as.string_val != NULL) {
                dst->type = INTEROP_TYPE_NULL;
                return -1;
            }
            break;
        case INTEROP_TYPE_NUMBER:
            dst->as.number_val = src->as.number_val;
            break;
        case INTEROP_TYPE_INTEGER:
            dst->as.integer_val = src->as.integer_val;
            break;
        case INTEROP_TYPE_BOOLEAN:
            dst->as.boolean_val = src->as.boolean_val;
            break;
        case INTEROP_TYPE_ARRAY:
            dst->as.array_val = src->as.array_val;
            break;
        case INTEROP_TYPE_ERROR:
            dst->as.error_val = src->as.error_val;
            break;
        default:
            break;
    }
    return 0;
}
