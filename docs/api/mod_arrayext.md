# C17 API Reference: Array Functional Extensions (`module/arrayext.h`)

## 1. Subsystem Overview & Responsibilities

The Array Functional Extensions Module (`module/arrayext.h`, implemented in `engine/src/module/arrayext.c`) provides high-performance functional programming transformations, vector operations, filtering, reduction, and aggregations across BASIC++ multi-dimensional arrays.

Key architectural responsibilities include:
- **Vectorized Mapping (`ARRAY MAP`)**: Transforms elements from a source array into a destination array using an inline expression, user-defined function, or subroutine callback (`arrayext_execute_map()`).
- **Array Filtering (`ARRAY FILTER`)**: Selects elements matching a predicate condition and compresses them into a target result array (`arrayext_func_filter()`).
- **Reduction & Aggregation (`ARRAY REDUCE` / `ARRAY AGGREGATE`)**: Computes statistical aggregates across arrays, including Minimum (`AGG_MIN`), Maximum (`AGG_MAX`), Sum (`AGG_SUM`), Average, and dot-product matrix reductions.
- **Cache-Optimized Loop Execution**: Bypasses statement loop overhead by executing vectorized transformations in tight C17 iteration loops over `BppArray->elements`.

## 2. Header Inclusion & Prerequisites

```c
#include "module/arrayext.h"
#include "runtime/arrays.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
/* Aggregation Operator Constants */
#define AGG_MIN 1
#define AGG_MAX 2
#define AGG_SUM 3
```

## 4. Function Prototypes & Operational Contracts

### Statement-Based Vector Execution
```c
/**
 * @brief Executes an ARRAY MAP statement transformation.
 * @param vm Pointer to active VMContext.
 * @param src_arr Source array identifier.
 * @param dst_arr Destination array identifier.
 * @param fn_name Optional function name to apply.
 * @param label_name Optional subroutine label callback.
 * @param expr_str Optional inline expression string.
 * @return BppError detailing execution status.
 */
BppError arrayext_execute_map(
    VMContext  *vm,
    const char *src_arr,
    const char *dst_arr,
    const char *fn_name,
    const char *label_name,
    const char *expr_str
);
```

### Functional Expression Operators
```c
/**
 * @brief Evaluates an ARRAY_MAP() function expression.
 */
BValue arrayext_func_map(VMContext *vm, LexerContext *lex, BppError *err);

/**
 * @brief Evaluates an ARRAY_FILTER() function expression.
 */
BValue arrayext_func_filter(VMContext *vm, LexerContext *lex, BppError *err);

/**
 * @brief Evaluates an ARRAY_REDUCE() function expression.
 */
BValue arrayext_func_reduce(VMContext *vm, LexerContext *lex, BppError *err);

/**
 * @brief Evaluates an aggregate calculation (MIN, MAX, SUM).
 */
BValue arrayext_func_aggregate(VMContext *vm, LexerContext *lex, int agg_type, BppError *err);
```

## 5. Architectural Invariants

- **Dimension Compatibility**: In mapping operations, destination arrays must match the dimension capacity of the source array.
- **Reference Safety**: Temporary string elements created during transformation are retained and released through `StringContext`.

## 6. Code Example: Functional Array Operations in BASIC++

```basic
10 DIM NUMS(5)
20 FOR I = 1 TO 5 : NUMS(I) = I * 10 : NEXT I
30 DIM DOUBLED(5)
40 ARRAY MAP NUMS INTO DOUBLED USING "X * 2"
50 PRINT "Sum of doubled numbers: "; ARRAY_SUM(DOUBLED)
```
