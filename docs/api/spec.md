# C17 API Reference: Language Specification Registry (`runtime/spec.h`)

## 1. Subsystem Overview & Responsibilities

The Language Specification Registry Subsystem (`runtime/spec.h`, implemented in `engine/src/runtime/spec.c`) provides declarative language specification loading (`.spec`, `.yaml`), companion micro-library binding, keyword metadata mapping, and dialect metaprogramming for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Dialect Metaprogramming**: Enables BASIC++ to define, validate, generate, and document its own language specifications natively via declarative metadata blocks and specification files.
- **Specification Classification (`SpecCategory`)**:
  - `SPEC_CAT_STATEMENT`: Language statement specifications.
  - `SPEC_CAT_FUNCTION`: Built-in and user-defined function specifications.
- **Companion Micro-Library Dynamic Linkage**: Automatically discovers and loads companion shared libraries associated with custom language specifications (`spec_load_companion_libraries()`).
- **Inline Metadata Registration**: Parses inline specification metadata declared directly within BASIC++ source scripts.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/spec.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
#define MAX_SPECS 128

typedef enum {
    SPEC_CAT_STATEMENT = 0,
    SPEC_CAT_FUNCTION
} SpecCategory;

/* Specification Object Descriptor */
typedef struct {
    char         name[64];              /* Specification name (e.g. "GRAPHICS.RASTER") */
    char         version[16];           /* Version string (e.g. "1.0") */
    SpecCategory category;              /* STATEMENT or FUNCTION */
    char         lib_path[256];         /* Path to backing companion micro-library */
    char         required_level[32];    /* Required security level string */
    BppKeywordId kw_id;                 /* Associated keyword enum */
    bool         lib_loaded;            /* True if companion library is currently active */
} SpecObject;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the specification registry tables.
 */
void spec_registry_init(void);

/**
 * @brief Loads a specification definition from an external .spec or .yaml file.
 */
int spec_load_file(VMContext *vm, const char *filename);

/**
 * @brief Loads all companion shared libraries for registered specifications.
 */
int spec_load_companion_libraries(VMContext *vm, const char *dir_part);

/**
 * @brief Registers an inline specification from a metadata block.
 */
int spec_register_inline(VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level);

/**
 * @brief Looks up a specification object by name.
 */
SpecObject *spec_find_by_name(const char *name);

/**
 * @brief Looks up a specification object by its assigned keyword ID.
 */
SpecObject *spec_find_by_kw_id(BppKeywordId kw_id);
```

## 5. Architectural Invariants

- **Isolated Specification Tables**: Stored in bounded arrays (`MAX_SPECS = 128`) without polluting core VM instruction tables.

## 6. Code Example: Registering an Inline Language Specification

```c
#include "runtime/spec.h"

void register_crypto_spec(VMContext *vm) {
    spec_register_inline(
        vm,
        "CRYPTO.SHA256",
        SPEC_CAT_FUNCTION,
        "libcrypto.so",
        "SEC_SAFE"
    );
}
```
