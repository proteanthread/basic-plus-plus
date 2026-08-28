// FILENAME: arrays.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libext
// NEEDS: libcore (memory.h, memory.c, strings.h, strings.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for arrays within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_ARRAYS_H
#define RUNTIME_ARRAYS_H

#include <stdbool.h>

#include "types/types.h"
#include "memory/memory.h"
#include "runtime/strings.h"

// Opaque Array Subsystem Context
struct VariableContext;
typedef struct ArrayContext ArrayContext;

// @brief Initialize and shut down array context.
ArrayContext *arr_init(MemoryContext *mem, StringContext *str);
void          arr_shutdown(ArrayContext *ctx);

// @brief Clear and delete all currently active arrays.
void          arr_clear_all(ArrayContext *ctx);
void          arr_clear_for_chain(ArrayContext *ctx, struct VariableContext *var_ctx);

// @brief Declare a new multi-dimensional array (DIM statement).
// @return BppError detailing success or failures (e.g. Redimensioned array, Out of memory).
BppError      arr_dim(ArrayContext *ctx, const char *name, int num_dims, const int *bounds);
void          arr_set_type(ArrayContext *ctx, const char *name, ValueType type);

// @brief Erase a specific array from the registry (ERASE statement).
// @return true on success, false if array was not found.
bool          arr_erase(ArrayContext *ctx, const char *name);

// @brief Get the pointer to a specific array element.
// @return Pointer to target BValue, or NULL if out of bounds or not found.
BValue       *arr_get_element(ArrayContext *ctx, const char *name, int num_dims, const int *indices, BppError *err);
bool          arr_exists(ArrayContext *ctx, const char *name);

// @brief Retrieve upper bound of a specific array dimension.
int           arr_ubound(ArrayContext *ctx, const char *name, int dimension, bool *out_found);

// @brief Option Base management APIs.
void          arr_set_option_base(ArrayContext *ctx, int base);
int           arr_get_option_base(ArrayContext *ctx);

// @brief Retrieve internal details for matrix/MAT operations.
int           arr_get_dimensions(ArrayContext *ctx, const char *name, int *out_bounds, int max_dims);
BValue       *arr_get_flat_elements(ArrayContext *ctx, const char *name, int *out_total_size);
BValue       *arr_ensure_capacity(ArrayContext *ctx, const char *name, int min_elements);

double        arr_get_last_det(ArrayContext *ctx);
void          arr_set_last_det(ArrayContext *ctx, double val);

bool arr_serialize(ArrayContext *ctx, void *fp);
bool arr_deserialize(ArrayContext *ctx, void *fp);

bool arr_create_alias(ArrayContext *ctx, const char *alias_name, const char *target_name);
void arr_remove_alias(ArrayContext *ctx, const char *alias_name);

BppError      arr_dim_virtual(ArrayContext *ctx, const char *name, int num_dims, const int *bounds, int channel);
int           arr_get_channel(ArrayContext *ctx, const char *name);
void          arr_set_channel(ArrayContext *ctx, const char *name, int channel);

#endif // RUNTIME_ARRAYS_H
