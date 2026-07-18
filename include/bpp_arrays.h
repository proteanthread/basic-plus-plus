/**
 * @file bpp_arrays.h
 * @brief Dynamic Array Management Subsystem API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares interfaces for declaring, retrieving, and freeing
 *   multi-dimensional arrays of BValue structs.
 * - Why it exists: Fulfills the Phase 2 core array requirements (DIM, ERASE).
 * - Why it works this way: It maintains array descriptors privately in an ArrayContext
 *   using uppercase-normalized hash maps. Elements are stored flat on the heap and indexed
 *   using multi-dimensional coordinate multipliers.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Maximum dimensions limits (currently 4), lower bound defaults (OPTION BASE).
 * - What cannot be changed: Memory clear guarantees on array erasure.
 * - What to expect: Accessing indexes outside bounds returns NULL and sets BppError.
 * - What to do if something breaks: Trace flat index calculations in arrays.c.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Index bounds fit in standard integers.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Support REDIM (dynamic resizing) or boundary checks customization.
 * - How to write external extensions: Plugins query array elements using arr_get_element.
 */

#ifndef BPP_ARRAYS_H
#define BPP_ARRAYS_H

#include "bpp_types.h"
#include "bpp_memory.h"
#include "bpp_strings.h"

/* Opaque Array Subsystem Context */
typedef struct ArrayContext ArrayContext;

/**
 * @brief Initialize and shut down array context.
 */
ArrayContext *arr_init(MemoryContext *mem, StringContext *str);
void          arr_shutdown(ArrayContext *ctx);

/**
 * @brief Clear and delete all currently active arrays.
 */
void          arr_clear_all(ArrayContext *ctx);

/**
 * @brief Declare a new multi-dimensional array (DIM statement).
 * @return BppError detailing success or failures (e.g. Redimensioned array, Out of memory).
 */
BppError      arr_dim(ArrayContext *ctx, const char *name, int num_dims, const int *bounds);
void          arr_set_type(ArrayContext *ctx, const char *name, ValueType type);

/**
 * @brief Erase a specific array from the registry (ERASE statement).
 * @return true on success, false if array was not found.
 */
bool          arr_erase(ArrayContext *ctx, const char *name);

/**
 * @brief Get the pointer to a specific array element.
 * @return Pointer to target BValue, or NULL if out of bounds or not found.
 */
BValue       *arr_get_element(ArrayContext *ctx, const char *name, int num_dims, const int *indices, BppError *err);
bool          arr_exists(ArrayContext *ctx, const char *name);

/**
 * @brief Retrieve upper bound of a specific array dimension.
 */
int           arr_ubound(ArrayContext *ctx, const char *name, int dimension, bool *out_found);

/**
 * @brief Option Base management APIs.
 */
void          arr_set_option_base(ArrayContext *ctx, int base);
int           arr_get_option_base(ArrayContext *ctx);

/**
 * @brief Retrieve internal details for matrix/MAT operations.
 */
int           arr_get_dimensions(ArrayContext *ctx, const char *name, int *out_bounds, int max_dims);
BValue       *arr_get_flat_elements(ArrayContext *ctx, const char *name, int *out_total_size);

double        arr_get_last_det(ArrayContext *ctx);
void          arr_set_last_det(ArrayContext *ctx, double val);

bool arr_serialize(ArrayContext *ctx, void *fp);
bool arr_deserialize(ArrayContext *ctx, void *fp);

#endif /* BPP_ARRAYS_H */
