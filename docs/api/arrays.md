# C17 API Reference: Array Subsystem (`runtime/arrays.h`)

## 1. Subsystem Overview & Responsibilities

The Array Subsystem (`runtime/arrays.h`, implemented in `engine/src/runtime/arrays.c` and `engine/src/runtime/mathext.c`) provides multi-dimensional array allocation, dynamic dimension descriptors, bounds checking, element indexing, and matrix mathematics for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Dynamic Dimension Descriptors**: Supports up to 8 dimensions per array with runtime-configured lower and upper bounds.
- **OPTION BASE Compliance**: Strictly respects `OPTION BASE 0` or `OPTION BASE 1` across all indexing and dimension allocation functions via `arr_get_option_base()`.
- **Reference-Counted Elements**: Manages arrays of numeric values (`VAL_NUMBER`), strings (`VAL_STRING`), and user-defined `TYPE` structures (`VAL_STRUCT`), properly retaining and releasing string elements through `StringContext`.
- **Fast Linearized Offset Mapping**: Computes multi-dimensional row-major linear byte offsets with $O(1)$ arithmetic complexity.
- **Array Clearing & Chaining**: Supports erasing individual arrays (`ERASE`), clearing all non-common arrays for chained programs (`arr_clear_for_chain()`), and global destruction (`arr_clear_all()`).

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/arrays.h"
#include "memory/memory.h"
#include "runtime/strings.h"
```

## 3. Data Structures & Types

```c
/* Opaque handle to the Array Subsystem Context */
typedef struct ArrayContext ArrayContext;

/* Internal Array Descriptor Layout (engine/src/runtime/arrays.c) */
typedef struct {
    char         name[64];          /* Normalized uppercase array identifier */
    ValueType    element_type;      /* VAL_NUMBER, VAL_STRING, or VAL_STRUCT */
    int          num_dims;          /* Number of dimensions (1 to 8) */
    int          bounds[8];         /* Upper bound for each dimension */
    int          lower_bounds[8];   /* Lower bound (0 or 1 based on OPTION BASE) */
    size_t       total_elements;    /* Precomputed total element count */
    BValue      *elements;          /* Contiguous heap-allocated BValue array */
    bool         is_common;         /* Preserved across CHAIN if declared COMMON */
} BppArray;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle Management
```c
/**
 * @brief Initializes the Array Subsystem Context.
 * @param mem Pointer to active MemoryContext.
 * @param str Pointer to active StringContext for string element refcounting.
 * @return Allocated ArrayContext pointer, or NULL on out-of-memory.
 */
ArrayContext *arr_init(MemoryContext *mem, StringContext *str);

/**
 * @brief Destroys the ArrayContext, releasing all element storage and strings.
 */
void arr_shutdown(ArrayContext *ctx);

/**
 * @brief Clears and frees all registered arrays.
 */
void arr_clear_all(ArrayContext *ctx);

/**
 * @brief Clears non-COMMON arrays when chaining to a new program.
 */
void arr_clear_for_chain(ArrayContext *ctx, struct VariableContext *var_ctx);
```

### Dimensioning & Erasing
```c
/**
 * @brief Dimensions a new multi-dimensional array (DIM statement).
 * @param ctx Array context.
 * @param name Array identifier (e.g. "GRID%", "NAMES$").
 * @param num_dims Number of dimensions (1..8).
 * @param bounds Array of upper bounds for each dimension.
 * @return BppError (Error 10 "Duplicate definition" if already dim'd).
 */
BppError arr_dim(ArrayContext *ctx, const char *name, int num_dims, const int *bounds);

/**
 * @brief Erases an existing array and frees its elements (ERASE statement).
 * @return true if successfully erased, false if array was not found.
 */
bool arr_erase(ArrayContext *ctx, const char *name);
```

### Element Access & Querying
```c
/**
 * @brief Retrieves a pointer to a specific array element for reading or writing.
 * @param ctx Array context.
 * @param name Array identifier.
 * @param num_dims Number of index arguments passed.
 * @param indices Array of 0-based or 1-based index integers.
 * @param err Pointer to BppError structure receiving boundary error codes (Error 9).
 * @return Pointer to target BValue, or NULL on error.
 */
BValue *arr_get_element(ArrayContext *ctx, const char *name, int num_dims, const int *indices, BppError *err);

/**
 * @brief Checks if an array with the specified name exists in the registry.
 */
bool arr_exists(ArrayContext *ctx, const char *name);

/**
 * @brief Retrieves the upper bound of a specific dimension (UBOUND function).
 */
int arr_ubound(ArrayContext *ctx, const char *name, int dimension, bool *out_found);

/**
 * @brief Option Base management APIs.
 */
void arr_set_option_base(ArrayContext *ctx, int base);
int  arr_get_option_base(ArrayContext *ctx);
```

## 5. Memory Safety & Reference-Count Invariants

- **String Element Ownership**: When overwriting an existing `VAL_STRING` element in an array, the previous string handle MUST be released via `str_release(str_ctx, old_val.as.string)` before storing the retained replacement.
- **Option Base Guard**: Loops iterating across array elements must never hardcode starting indices `0` or `1`; always query `arr_get_option_base(ctx)`.

## 6. Code Example: Dimensioning and Accessing a 2D Array

```c
#include "runtime/arrays.h"
#include <stdio.h>

void example_2d_array(ArrayContext *arr_ctx) {
    int bounds[2] = {10, 20}; /* 10 rows, 20 columns */
    BppError err = arr_dim(arr_ctx, "MATRIX", 2, bounds);
    if (err.code != 0) {
        printf("DIM failed: %s\n", err.message);
        return;
    }

    /* Assign value at row 3, col 5 */
    int indices[2] = {3, 5};
    BValue *elem = arr_get_element(arr_ctx, "MATRIX", 2, indices, &err);
    if (elem) {
        elem->type = VAL_NUMBER;
        elem->as.number = 42.0;
    }
}
```
