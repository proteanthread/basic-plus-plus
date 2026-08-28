# C17 API Reference: Feature Registry & Runtime State (`core/feature_reg.h`)

## 1. Subsystem Overview & Responsibilities

The Feature Registry & Runtime State Subsystem (`core/feature_reg.h`, implemented in `engine/src/core/feature_reg.c`) provides central registration, runtime feature discovery, built-in function catalogs, keyword tables, and interactive help topic lookup for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Central Feature Introspection**: Tracks up to 512 registered statement keywords (`FeatureKeywordEntry`), 512 built-in functions (`FeatureBuiltinEntry`), and 512 help topics (`FeatureHelpEntry`).
- **Dynamic Introspection Queries**: Exposes fast lookup functions by name or numeric index to power runtime `CATALOG`, `HELP`, and language discovery statements.
- **Categorization & Taxonomies**: Organizes keywords and functions into standardized categories (e.g. "Control Flow", "File I/O", "Graphics System", "String Processing", "Game Engines").
- **Freestanding C17 Compliance**: Implemented using fixed-size static tables with zero dynamic heap allocation overhead.

## 2. Header Inclusion & Prerequisites

```c
#include "core/feature_reg.h"
#include <stddef.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
#define MAX_FEATURE_KEYWORDS     512
#define MAX_FEATURE_BUILTINS     512
#define MAX_FEATURE_HELP_ENTRIES 512

/* Feature Descriptor for Statement Keywords */
typedef struct {
    char        name[32];       /* Uppercase keyword name (e.g. "PRINT") */
    int         token_id;       /* Associated token enum ID */
    const char *category;       /* Category string (e.g. "Console I/O") */
} FeatureKeywordEntry;

/* Feature Descriptor for Builtin Functions */
typedef struct {
    char        name[32];       /* Uppercase function name (e.g. "SIN") */
    const char *help_text;      /* Concise help summary */
    const char *category;       /* Category string (e.g. "Math") */
} FeatureBuiltinEntry;

/* Feature Descriptor for HELP Topics */
typedef struct {
    char        topic[32];      /* Topic identifier */
    const char *summary;        /* Summary description */
    const char *usage;          /* Syntax and usage examples */
    const char *category;       /* Category string */
} FeatureHelpEntry;
```

## 4. Function Prototypes & Operational Contracts

### Registration APIs
```c
/**
 * @brief Initializes the feature registry tables.
 */
void feature_reg_init(void);

bool feature_register_keyword(const char *keyword, int token_id, const char *category);
bool feature_register_builtin(const char *name, const char *help_text, const char *category);
bool feature_register_help(const char *topic, const char *summary, const char *usage, const char *category);
```

### Introspection & Lookup APIs
```c
size_t                     feature_get_keyword_count(void);
const FeatureKeywordEntry *feature_get_keyword_by_index(size_t index);
const FeatureKeywordEntry *feature_find_keyword(const char *name);

size_t                     feature_get_builtin_count(void);
const FeatureBuiltinEntry *feature_get_builtin_by_index(size_t index);
const FeatureBuiltinEntry *feature_find_builtin(const char *name);

size_t                     feature_get_help_count(void);
const FeatureHelpEntry    *feature_get_help_by_index(size_t index);
const FeatureHelpEntry    *feature_find_help(const char *topic);
```

## 5. Architectural Invariants

- **Zero Allocation Registry**: Uses fixed-size arrays to ensure zero memory fragmentation and fast $O(1)$/$O(N)$ lookups.

## 6. Code Example: Querying Registered Features in C

```c
#include "core/feature_reg.h"
#include <stdio.h>

void print_registered_keywords(void) {
    size_t count = feature_get_keyword_count();
    printf("Total registered keywords: %zu\n", count);
    for (size_t i = 0; i < count; i++) {
        const FeatureKeywordEntry *k = feature_get_keyword_by_index(i);
        printf("  %s (%s)\n", k->name, k->category);
    }
}
```
