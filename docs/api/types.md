# C17 API Reference: Core Data Types & Values (`types/types.h`)

## 1. Subsystem Overview & Responsibilities

The Core Data Types & Values Header (`types/types.h`) defines the fundamental data representations, tagged union `BValue` types, fractional line number representations (`BppLineNumber`), and VM bytecode opcodes for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Universal Value Representation (`BValue`)**: A memory-compact tagged union representing numbers (`VAL_NUMBER`), integers (`VAL_INTEGER`), reference-counted strings (`VAL_STRING`), associative dictionaries (`VAL_MAP`), array references (`VAL_ARRAY_REF`), and `FIELD`-bound file buffer strings (`VAL_FIELD_STRING`).
- **64-Bit Floating-Point Precision**: Stores numeric values as double-precision IEEE 754 floating-point numbers (`double`).
- **Fractional Line Numbers (`BppLineNumber`)**: Uses `double` to support fractional line numbers (e.g. `10.5`) commonly found in classic Dartmouth BASIC and Tymshare Super BASIC dialects.
- **Virtual Machine Opcodes (`BppOpcode`)**: Declares standard bytecode instructions for the interpreter VM loop.

## 2. Header Inclusion & Prerequisites

```c
#include "types/types.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
/* Forward Declarations */
typedef struct BppString BppString;
typedef BppString* BppStringRef;
typedef struct BppMap BppMap;

/* Value Type Identifiers */
typedef enum {
    VAL_NONE         = 0,  /* Uninitialized state */
    VAL_NUMBER       = 1,  /* Universal double-precision float */
    VAL_INTEGER      = 2,  /* Optimized integer representation */
    VAL_STRING       = 3,  /* Reference-counted string handle */
    VAL_MAP          = 4,  /* Reference-counted dictionary/map */
    VAL_ARRAY_REF    = 5,  /* Symbolic link to ArrayContext array */
    VAL_FIELD_STRING = 6   /* GW-BASIC Random Access File bound string */
} ValueType;

/* The Primary Tagged-Union Value Struct */
typedef struct {
    ValueType type;
    union {
        double        number;       /* Floats, integers, booleans */
        BppStringRef  string;       /* Reference to isolated string heap */
        BppMap       *map;          /* Reference-counted map */
        const char   *array_name;   /* Array identifier */
        struct {
            int channel;
            int offset;
            int length;
        } field_str;                /* Bound to random-access file buffer */
    } as;
} BValue;

/* Fractional Line Number Type */
typedef double BppLineNumber;
```

## 4. Helper Constructors & Accessors

```c
static inline BValue bval_number(double n) {
    BValue v; v.type = VAL_NUMBER; v.as.number = n; return v;
}

static inline BValue bval_string(BppStringRef s) {
    BValue v; v.type = VAL_STRING; v.as.string = s; return v;
}

static inline bool bval_is_truthy(const BValue *val) {
    if (!val) return false;
    if (val->type == VAL_NUMBER) return val->as.number != 0.0;
    if (val->type == VAL_STRING) return val->as.string != NULL;
    return false;
}
```

## 5. Architectural Invariants

- **Memory Safety**: Direct access to union members without checking `val.type` is strictly forbidden.
- **Deterministic Zero Initialization**: All `BValue` structs must be initialized (`memset` or `{0}`) before assignment.

## 6. Code Example: Creating and Inspecting BValue in C

```c
#include "types/types.h"
#include <stdio.h>

void inspect_value(const BValue *v) {
    switch (v->type) {
        case VAL_NUMBER:
            printf("Number: %f\n", v->as.number);
            break;
        case VAL_STRING:
            printf("String handle present\n");
            break;
        default:
            printf("Other/Null type\n");
            break;
    }
}
```
