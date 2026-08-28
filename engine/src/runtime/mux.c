// FILENAME: mux.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pack.c, unpack.c)
// NEEDS: libcore (feature_reg.h, feature_reg.c, string.h)
// NEEDS: libengine (mux.h, string.c)
// Provides core logic and interface definitions for mux within BASIC++.
//
// ---- Includes ----

#include "runtime/mux.h"
#include "core/feature_reg.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void mux_subsystem_init(void) {
    feature_register_keyword("MUX", 901, "Extended");
    feature_register_keyword("DEMUX", 902, "Extended");
    feature_register_builtin("PACK$", "Packs multiple values or arrays into a binary string buffer.", "String");
    feature_register_builtin("UNPACK", "Unpacks a binary string buffer into target variables or arrays.", "String");
    feature_register_help("MUX", "Interleaves multiple arrays, matrices, or streams into a single composite destination.", "MUX target, arr1, arr2 [, STRIDE s]", "Extended");
    feature_register_help("DEMUX", "De-interleaves a composite array, matrix, or stream into separate outputs.", "DEMUX arr1, arr2 FROM src [, STRIDE s]", "Extended");
    feature_register_help("PACK$", "Serializes variables or arrays into a packed binary string buffer.", "b$ = PACK$(val1, val2, arr())", "String");
    feature_register_help("UNPACK", "Deserializes a packed binary string buffer into variables or arrays.", "UNPACK b$, val1, val2, arr()", "String");
}

bool mux_arrays(DimArray *dst, DimArray **srcs, size_t count, MuxMode mode, size_t stride) {
    size_t i, j, src_idx;
    size_t step = (stride > 0) ? stride : 1;

    if (!dst || !srcs || count == 0 || !dst->data) return false;

    memset(dst->data, 0, (size_t)dst->total_size * sizeof(BValue));

    // Interleave element-by-element
    j = 0;
    for (i = 0; ; i += step) {
        bool added_any = false;
        for (src_idx = 0; src_idx < count; src_idx++) {
            DimArray *src = srcs[src_idx];
            if (src && src->data) {
                size_t k;
                for (k = 0; k < step; k++) {
                    size_t elem_idx = i + k;
                    if (elem_idx < (size_t)src->total_size) {
                        if (j < (size_t)dst->total_size) {
                            dst->data[j] = src->data[elem_idx];
                            if (dst->data[j].type == VAL_STRING && dst->data[j].as.string) {
                                str_add_ref(dst->data[j].as.string);
                            }
                            j++;
                        }
                        added_any = true;
                    }
                }
            }
        }
        if (!added_any) break;
    }

    return true;
}

bool demux_arrays(DimArray **dsts, size_t count, DimArray *src, MuxMode mode, size_t stride) {
    size_t i, src_idx, j;
    size_t step = (stride > 0) ? stride : 1;

    if (!dsts || count == 0 || !src || !src->data) return false;

    j = 0;
    for (i = 0; j < (size_t)src->total_size; i += step) {
        for (src_idx = 0; src_idx < count; src_idx++) {
            DimArray *dst = dsts[src_idx];
            if (dst && dst->data) {
                size_t k;
                for (k = 0; k < step; k++) {
                    if (j < (size_t)src->total_size) {
                        size_t dst_idx = i + k;
                        if (dst_idx < (size_t)dst->total_size) {
                            dst->data[dst_idx] = src->data[j];
                            if (dst->data[dst_idx].type == VAL_STRING && dst->data[dst_idx].as.string) {
                                str_add_ref(dst->data[dst_idx].as.string);
                            }
                        }
                        j++;
                    }
                }
            }
        }
    }

    return true;
}

bool mat_mux_execute(DimArray *dst, DimArray *src_a, DimArray *src_b, MuxMode mode) {
    DimArray *srcs[2];
    srcs[0] = src_a;
    srcs[1] = src_b;
    return mux_arrays(dst, srcs, 2, mode, 1);
}

bool mat_demux_execute(DimArray *dst_a, DimArray *dst_b, DimArray *src, MuxMode mode) {
    DimArray *dsts[2];
    dsts[0] = dst_a;
    dsts[1] = dst_b;
    return demux_arrays(dsts, 2, src, mode, 1);
}

bool mux_bvalues(BValue *dst_arr, size_t *dst_len, const BValue *src1, size_t len1, const BValue *src2, size_t len2) {
    size_t i = 0, j = 0, k = 0;
    if (!dst_arr || !dst_len) return false;

    while (i < len1 || j < len2) {
        if (i < len1) {
            dst_arr[k++] = src1[i++];
        }
        if (j < len2) {
            dst_arr[k++] = src2[j++];
        }
    }
    *dst_len = k;
    return true;
}

char* pack_fields(const BValue *values, size_t count, size_t *out_len) {
    size_t total_bytes = 0;
    size_t i;
    char *buf;
    size_t offset = 0;

    if (!values || count == 0 || !out_len) return NULL;

    // First pass: calculate total byte size required
    for (i = 0; i < count; i++) {
        total_bytes += 1; // Type tag byte
        if (values[i].type == VAL_NUMBER) {
            total_bytes += sizeof(double);
        } else if (values[i].type == VAL_STRING) {
            size_t slen = values[i].as.string ? str_len(values[i].as.string) : 0;
            total_bytes += sizeof(uint32_t) + slen;
        }
    }

    buf = (char*)calloc(total_bytes + 1, sizeof(char));
    if (!buf) return NULL;

    // Second pass: encode type tags and data
    for (i = 0; i < count; i++) {
        buf[offset++] = (char)values[i].type;
        if (values[i].type == VAL_NUMBER) {
            memcpy(buf + offset, &values[i].as.number, sizeof(double));
            offset += sizeof(double);
        } else if (values[i].type == VAL_STRING) {
            const char *cstr = values[i].as.string ? str_data(values[i].as.string) : "";
            uint32_t slen = (uint32_t)(values[i].as.string ? str_len(values[i].as.string) : 0);
            memcpy(buf + offset, &slen, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            if (slen > 0) {
                memcpy(buf + offset, cstr, slen);
                offset += slen;
            }
        }
    }

    *out_len = offset;
    return buf;
}

bool unpack_fields(struct StringContext *str_ctx, const char *buf, size_t buf_len, BValue *out_values, size_t count) {
    size_t offset = 0;
    size_t i;

    if (!buf || buf_len == 0 || !out_values || count == 0) return false;

    for (i = 0; i < count && offset < buf_len; i++) {
        uint8_t type_tag = (uint8_t)buf[offset++];
        out_values[i].type = (ValueType)type_tag;
        if (type_tag == VAL_NUMBER) {
            if (offset + sizeof(double) <= buf_len) {
                memcpy(&out_values[i].as.number, buf + offset, sizeof(double));
                offset += sizeof(double);
            } else {
                return false;
            }
        } else if (type_tag == VAL_STRING) {
            uint32_t slen = 0;
            if (offset + sizeof(uint32_t) <= buf_len) {
                memcpy(&slen, buf + offset, sizeof(uint32_t));
                offset += sizeof(uint32_t);
                if (offset + slen <= buf_len) {
                    out_values[i].as.string = str_create(str_ctx, buf + offset, slen);
                    offset += slen;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    return true;
}
