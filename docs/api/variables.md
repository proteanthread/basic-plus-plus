# C17 API Reference: Variable Context & Symbol Table (`runtime/variables.h`)

## 1. Subsystem Overview & Responsibilities

The Variable Context Subsystem (`runtime/variables.h`, implemented in `engine/src/runtime/variables.c`) manages scalar variable bindings, implicit/explicit declarations (`OPTION EXPLICIT`), variable scopes (`SHARED`, `STATIC`, local prefix frames), and preserved `COMMON` variables across program chaining for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Symbol Table Management**: Maps normalized variable identifiers (e.g. `A%`, `SUM#`, `TITLE$`) to tagged union `BValue` storage.
- **Scope Frames & Prefix Scoping**: Manages local variable frames within `SUB` and `FUNCTION` procedures (`var_set_scope()`, `var_clear_scope()`) while supporting `SHARED` global access.
- **COMMON Variable Preservation**: Marks variables declared via `COMMON` to survive `CHAIN` and `RUN "file", R` context resets (`var_clear_for_chain()`).
- **OPTION EXPLICIT Enforcement**: Rejects undeclared variable assignments with Error 100 when strict explicit variable checking is active.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/variables.h"
#include "memory/memory.h"
#include "runtime/strings.h"
```

## 3. Data Structures & Types

```c
/* Opaque Variable Context Handle */
typedef struct VariableContext VariableContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle
```c
VariableContext *var_init(MemoryContext *mem, StringContext *str);
void             var_shutdown(VariableContext *ctx);
```

### Variable Lookup & Declaration
```c
/**
 * @brief Looks up a variable by name.
 * @param name Normalized uppercase variable name (e.g. "COUNT%", "NAME$").
 * @param create_if_missing If true, declares the variable if not present.
 * @return Pointer to target BValue, or NULL if not found and create_if_missing is false.
 */
BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing);

/**
 * @brief Explicitly declares a variable (bypasses OPTION EXPLICIT check).
 */
BValue *var_declare(VariableContext *ctx, const char *name);

/**
 * @brief Assigns a new value to a variable, performing type validation.
 */
bool var_assign(VariableContext *ctx, const char *name, BValue val);
```

### Scope & Lifetime Management
```c
/**
 * @brief Clears all variables (CLEAR / NEW statements).
 */
void var_clear_all(VariableContext *ctx);

/**
 * @brief Clears all non-COMMON variables during program chaining.
 */
void var_clear_for_chain(VariableContext *ctx);

void var_mark_common(VariableContext *ctx, const char *name);
bool var_is_common(VariableContext *ctx, const char *name);

void var_set_scope(VariableContext *ctx, const char *scope);
void var_clear_scope(VariableContext *ctx, const char *prefix);
void var_set_shared(VariableContext *ctx, const char *name);
```

## 5. Architectural Invariants

- **String Retention**: When overwriting a string variable (`VAL_STRING`), the previous string reference MUST be released via `str_release(str_ctx, old_str)` before storing the new reference.
- **COMMON Retention**: `var_clear_for_chain()` preserves all variables flagged via `var_mark_common()`.

## 6. Code Example: Declaring and Assigning Variables in C

```c
#include "runtime/variables.h"

void set_player_score(VariableContext *vc, int score) {
    BValue *score_var = var_lookup(vc, "SCORE%", true);
    if (score_var) {
        score_var->type = VAL_NUMBER;
        score_var->as.number = (double)score;
    }
}
```
