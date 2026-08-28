// FILENAME: mux.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mux.c, pack.c, unpack.c)
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libengine (eval.h, eval.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for mux within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_MUX_H
#define RUNTIME_MUX_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "types/types.h"
#include "eval/eval.h"
#include "runtime/arrays.h"

// DimArray structure definition for array interleaving
typedef struct DimArray {
    BValue *data;
    int total_size;
    int num_dims;
    int bounds[4];
} DimArray;

// Multiplexing Interleave Modes
typedef enum {
    MUX_MODE_ELEMENT = 0, // Element-by-element interleave (A0, B0, A1, B1...)
    MUX_MODE_ROW     = 1, // Matrix row-by-row interleave
    MUX_MODE_COL     = 2, // Matrix column-by-column interleave
    MUX_MODE_STRIDE  = 3  // Custom stride-based interleave
} MuxMode;


// Public C17 Multiplexing Algorithms
bool mux_arrays(DimArray *dst, DimArray **srcs, size_t count, MuxMode mode, size_t stride);
bool demux_arrays(DimArray **dsts, size_t count, DimArray *src, MuxMode mode, size_t stride);

// Matrix Specific Interleaving
bool mat_mux_execute(DimArray *dst, DimArray *src_a, DimArray *src_b, MuxMode mode);
bool mat_demux_execute(DimArray *dst_a, DimArray *dst_b, DimArray *src, MuxMode mode);

// Heterogeneous BValue Variant Array Interleaving
bool mux_bvalues(BValue *dst_arr, size_t *dst_len, const BValue *src1, size_t len1, const BValue *src2, size_t len2);

// Binary Field Serialization & Unpacking
char* pack_fields(const BValue *values, size_t count, size_t *out_len);
struct StringContext;
bool unpack_fields(struct StringContext *str_ctx, const char *buf, size_t buf_len, BValue *out_values, size_t count);

// Virtual Device Stream Channel Multiplexing
bool stream_mux_channels(int target_fd, const int *src_fds, size_t count);
bool stream_demux_channels(int source_fd, const int *dst_fds, size_t count);

// Self-registration initializer for MUX micro-library
void mux_subsystem_init(void);

#endif // RUNTIME_MUX_H
