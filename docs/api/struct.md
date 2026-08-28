# C17 API Reference: User-Defined TYPE & Struct Subsystem (`core/struct.h`)

## 1. Subsystem Overview & Responsibilities

The User-Defined TYPE & Struct Subsystem (`core/struct.h`, implemented in `engine/src/core/struct.c`) provides definition, validation, field offset layout, instance allocation, and binary memory copying for structured user-defined types (`TYPE ... END TYPE`) and record objects in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **User-Defined Type Declaration (`TYPE ... END TYPE`)**: Parses and registers structured records containing up to 64 named fields (`MAX_TYPE_FIELDS`).
- **Heterogeneous Field Support**: Supports numeric primitives (`INTEGER`, `LONG`, `SINGLE`, `DOUBLE`), fixed/variable strings (`STRING * n`), arrays, and nested user-defined types.
- **Instance Instantiation (`struct_instantiate`)**: Allocates structured instances mapped to associative map structures or contiguous binary record buffers.
- **Deep & Shallow Copying (`struct_copy_instance`)**: Implements deterministic record assignment (`LET rec1 = rec2`) with proper string reference retention.

## 2. Header Inclusion & Prerequisites

```c
#include "core/struct.h"
#include "types/types.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
#define MAX_TYPE_FIELDS  64
#define MAX_USER_TYPES   64

/* Field Descriptor inside a TYPE Definition */
typedef struct {
    char      name[64];             /* Field identifier (e.g. "X", "Salary#") */
    ValueType type;                 /* VAL_NUMBER, VAL_STRING, VAL_MAP */
    char      nested_type[64];      /* Target type name if nested UDT */
} BppUserTypeField;

/* Complete User-Defined Type Definition */
typedef struct {
    char             name[64];                  /* Type name (e.g. "Point3D", "Customer") */
    BppUserTypeField fields[MAX_TYPE_FIELDS];   /* Array of field definitions */
    int              field_count;               /* Number of fields declared */
    bool             is_class;                  /* True if CLASS instead of TYPE */
} BppUserTypeDef;

/* Type Registry Structure */
typedef struct BppTypeRegistry {
    BppUserTypeDef types[MAX_USER_TYPES];
    int            count;
} BppTypeRegistry;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the type registry.
 */
void struct_registry_init(BppTypeRegistry *reg);

/**
 * @brief Registers a new user-defined type definition.
 */
bool struct_register_type(BppTypeRegistry *reg, const BppUserTypeDef *def, char *err_buf, size_t err_len);

/**
 * @brief Searches for a registered type definition by name.
 */
const BppUserTypeDef *struct_find_type(const BppTypeRegistry *reg, const char *name);

/**
 * @brief Instantiates a new instance of a user-defined type.
 * @return Allocated BppMap representing the struct instance.
 */
BppMap *struct_instantiate(VMContext *vm, const BppTypeRegistry *reg, const char *type_name, char *err_buf, size_t err_len);

/**
 * @brief Performs deep copy of a struct instance into a destination instance.
 */
bool struct_copy_instance(VMContext *vm, BppMap *dst, BppMap *src, char *err_buf, size_t err_len);
```

## 5. Architectural Invariants

- **Field Name Uniqueness**: Duplicate field names within the same type definition trigger Error 10 (`ERR_DUPLICATE_DEFINITION`).
- **Recursive Nesting Guard**: Prevents direct circular nesting of types to avoid infinite instantiation recursion.

## 6. Code Example: Defining and Using a Struct in BASIC++

```basic
10 TYPE Point2D
20   X AS DOUBLE
30   Y AS DOUBLE
40 END TYPE
50 
60 DIM P AS Point2D
70 P.X = 100.5
80 P.Y = 250.75
90 PRINT "Point Coordinates: ("; P.X; ", "; P.Y; ")"
```
