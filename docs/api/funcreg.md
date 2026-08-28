# C17 API Reference: Function Registry (`runtime/funcreg.h`)

## 1. Subsystem Overview & Responsibilities

The Function Registry Subsystem (`runtime/funcreg.h`, implemented in `engine/src/runtime/funcreg.c`) provides registration, metadata indexing, argument boundary checking, safety level classification, and fast symbol dispatch for built-in and user-extended C functions in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **Centralized Function Table**: Maintains up to 128 registered native C functions (`MAX_FUNCTIONS`) with metadata descriptors.
- **Safety & Purity Classification**: Categorizes functions by safety levels:
  - `FSAFE_PURE`: Deterministic, side-effect free mathematical operations (e.g. `SIN`, `COS`, `ABS`).
  - `FSAFE_STATE`: Functions reading or modifying internal VM state (e.g. `RND`, `TIMER`, `CSRLIN`).
  - `FSAFE_IO`: Functions performing device or file I/O (e.g. `INPUT$`, `INP`).
  - `FSAFE_SYSTEM`: Privileged system calls subject to capability sandbox verification (`CAP_SYS`).
- **Dynamic Extensibility & Overriding**: Allows user modules to register custom functions at runtime or override existing implementations via `OVERRIDE`.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/funcreg.h"
#include "types/types.h"
```

## 3. Data Structures & Types

```c
typedef enum {
    FCAT_CORE   = 0,
    FCAT_MATH   = 1,
    FCAT_STRING = 2,
    FCAT_IO     = 3,
    FCAT_UTIL   = 4,
    FCAT_USER   = 5
} FuncCategory;

typedef enum {
    FRET_INT    = 0,
    FRET_FLOAT  = 1,
    FRET_STRING = 2,
    FRET_ANY    = 3
} FuncReturnType;

typedef enum {
    FSAFE_PURE   = 0,
    FSAFE_STATE  = 1,
    FSAFE_IO     = 2,
    FSAFE_SYSTEM = 3
} FuncSafety;

typedef BValue (*FuncHandler)(BValue *args, int argc, void *rt);

typedef struct {
    const char     *name;        /* Normalized uppercase function identifier (e.g. "SQR", "HEX$") */
    BppKeywordId    keyword;     /* Associated keyword enum or KW_NONE */
    FuncCategory    category;    /* FCAT_MATH, FCAT_STRING, etc. */
    FuncReturnType  ret_type;    /* Expected return type */
    int             min_args;    /* Minimum allowed argument count */
    int             max_args;    /* Maximum allowed argument count */
    FuncSafety      safety;      /* Safety classification */
    int             overridable; /* 1 if can be replaced via OVERRIDE */
    FuncHandler     handler;     /* Native C function pointer */
    const char     *help_text;   /* Interactive documentation string */
    const char     *module_name; /* Name of registering module */
} FunctionEntry;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the function registry and loads built-in mathematical/string functions.
 */
void funcreg_init(void);

/**
 * @brief Shuts down the registry and releases registered metadata tables.
 */
void funcreg_shutdown(void);

/**
 * @brief Registers a new native C function in the registry.
 * @return 0 on success, -1 if table is full (MAX_FUNCTIONS exceeded).
 */
int funcreg_register(const FunctionEntry *entry);

/**
 * @brief Searches for a registered function by name (case-insensitive).
 * @return Pointer to FunctionEntry, or NULL if not found.
 */
const FunctionEntry *funcreg_find(const char *name);

/**
 * @brief Returns total number of registered functions.
 */
int funcreg_count(void);

/**
 * @brief Retrieves function entry by zero-based index for CATALOG enumeration.
 */
const FunctionEntry *funcreg_get(int index);
```

## 5. Architectural Invariants

- **Argument Range Validation**: The evaluator checks `min_args <= argc <= max_args` prior to invoking `handler`, returning Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`) on mismatch.
- **Tagged Union Return**: Handlers must return valid `BValue` structs (`VAL_NUMBER`, `VAL_STRING`, or `VAL_ERROR`).

## 6. Code Example: Registering a Native Custom Math Function

```c
#include "runtime/funcreg.h"
#include <math.h>

static BValue custom_cube(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double x = args[0].as.number;
    return bval_float(x * x * x);
}

void register_cube_function(void) {
    FunctionEntry entry = {
        .name = "CUBE",
        .category = FCAT_MATH,
        .ret_type = FRET_FLOAT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_PURE,
        .overridable = 1,
        .handler = custom_cube,
        .help_text = "Computes the cube of a numeric value (X ^ 3).",
        .module_name = "math_ext"
    };
    funcreg_register(&entry);
}
```
