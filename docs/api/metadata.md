# C17 API Reference: Metadata & MicroLib Registry (`runtime/metadata.h`)

## 1. Subsystem Overview & Responsibilities

The Metadata & MicroLib Registry Subsystem (`runtime/metadata.h`, implemented in `engine/src/runtime/metadata.c`) provides cross-file global label registration, inline docstring introspection, declarative metadata block storage (`KEYWORD`, `SCOPE`, `ALIAS`), and micro-library registration for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **MicroLib Metadata Registration**: Registers micro-library keyword descriptors (`MicroLibMetadata`) providing syntax cards, category classifications, and error code mappings for interactive `HELP` and `CATALOG`.
- **Global Label Cross-File Mapping**: Indexes global code labels (`::label:`) across multiple source files, allowing cross-file calls (`GOSUB ::MODULE::INIT`) and background task spawning (`TASK ::WORKER`).
- **Docstring Introspection**: Associates structured documentation docstrings with procedures, variables, and keywords for runtime `HELP` and IDE tooltips.
- **Stateful Metadata Blocks**: Stores declarative metaprogramming definitions declared via `KEYWORD`, `SCOPE`, and `ALIAS` blocks.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/metadata.h"
#include "runtime/micro_lib_metadata.h"
```

## 3. Data Structures & Types

```c
/* Micro-Library Keyword Descriptor */
typedef struct {
    const char *name;           /* Primary keyword name (e.g. "PRINT", "SCREEN") */
    const char *category;       /* Category name (e.g. "Console I/O", "Graphics") */
    const char *syntax;         /* Syntax signature card */
    const char *help_text;      /* Descriptive help text */
    const char *error_codes;    /* Associated error code descriptions */
} MicroLibMetadata;

/* Global Cross-File Label Descriptor */
typedef struct {
    char          label_name[64];
    char          filename[256];
    BppLineNumber line_number;
} BppGlobalLabel;

/* Introspection Docstring Descriptor */
typedef struct {
    char target_name[128];      /* Identifier or label target */
    char docstring[256];        /* Extracted documentation text */
} BppDocstring;
```

## 4. Function Prototypes & Operational Contracts

### Micro-Library Registration
```c
/**
 * @brief Registers a micro-library keyword metadata descriptor block.
 */
void microlib_register(const MicroLibMetadata *meta);

/**
 * @brief Searches for registered micro-library metadata by keyword name.
 */
const MicroLibMetadata *microlib_lookup(const char *name);

/**
 * @brief Returns total number of registered micro-libraries.
 */
size_t microlib_count(void);
```

### Global Labels & Docstrings
```c
/**
 * @brief Registers a cross-file global label mapping.
 */
bool metadata_register_label(const char *name, const char *filename, BppLineNumber line);

/**
 * @brief Resolves a global label name to its defining filename and line number.
 */
bool metadata_find_label(const char *name, const char **out_filename, BppLineNumber *out_line);

/**
 * @brief Associates a docstring with a target identifier.
 */
bool metadata_register_docstring(const char *target, const char *docstring);

/**
 * @brief Retrieves docstring text for a target identifier.
 */
const char *metadata_find_docstring(const char *target);
```

## 5. Architectural Invariants

- **Introspection Parity**: Every new keyword or statement implemented in the C engine MUST call `microlib_register()` to guarantee interactive `HELP` and `CATALOG` parity.

## 6. Code Example: Registering Keyword Metadata in C

```c
#include "runtime/metadata.h"

void register_my_keyword_docs(void) {
    static const MicroLibMetadata meta = {
        .name = "MYKEYWORD",
        .category = "System Integration",
        .syntax = "MYKEYWORD [param1, param2]",
        .help_text = "Performs custom system integration tasks.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
```
