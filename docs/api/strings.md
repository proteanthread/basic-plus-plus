# C17 API Reference: String Pool & Memory Runtime (`runtime/strings.h`)

## 1. Subsystem Overview & Responsibilities

The String Pool & Memory Runtime Subsystem (`runtime/strings.h`, implemented in `engine/src/runtime/strings.c`) provides reference-counted string allocation, concatenation, slicing, length caching, and garbage collection for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Reference-Counted Heap Allocation**: Allocates immutable string payloads in an isolated string heap (`str_mem`), eliminating duplicate buffers and supporting zero-copy assignments.
- **Two-Parameter `str_release()` Invariant**: String release MUST be called with two arguments: `str_release(StringContext *ctx, BppStringRef ref)`. Passing a single argument is a compiler error.
- **Length-Prefixed UTF-8 / ASCII Support**: Stores precomputed string byte lengths, enabling $O(1)$ `LEN()` calculations and safe embedded null byte handling.
- **String Manipulation Primitives**: Provides high-speed concatenation (`str_concat`), slicing (`str_mid`), case transformation, and formatting primitives.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/strings.h"
#include "memory/memory.h"
#include "types/types.h"
```

## 3. Data Structures & Types

```c
/* Opaque String Context */
typedef struct StringContext StringContext;

/* Forward Declaration of String Handle */
typedef struct BppString BppString;
typedef BppString* BppStringRef;

/* Internal String Layout (engine/src/runtime/strings.c) */
struct BppString {
    int    ref_count;   /* Reference counter */
    size_t length;      /* String length in bytes */
    char   data[];      /* Flexible array member containing null-terminated text */
};
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle
```c
/**
 * @brief Initializes the string manager and allocates the string heap.
 */
StringContext *str_init(MemoryContext *mem);

/**
 * @brief Shuts down the string manager and deallocates all pooled strings.
 */
void str_shutdown(StringContext *ctx);
```

### String Creation & Lifecycle
```c
/**
 * @brief Creates a new reference-counted string from a raw buffer.
 * @param ctx String context pointer.
 * @param data Byte buffer containing text.
 * @param length Length of text in bytes.
 * @return Retained BppStringRef handle (ref_count initialized to 1).
 */
BppStringRef str_create(StringContext *ctx, const char *data, size_t length);

/**
 * @brief Increments reference count of a string handle.
 */
void str_add_ref(BppStringRef ref);

/**
 * @brief Decrements refcount and frees string when count reaches 0.
 * @note MUST pass StringContext as the first argument.
 */
void str_release(StringContext *ctx, BppStringRef ref);
```

### String Operations & Accessors
```c
/**
 * @brief Concatenates two strings and returns a new reference-counted string.
 */
BppStringRef str_concat(StringContext *ctx, BppStringRef a, BppStringRef b);

/**
 * @brief Extracts a substring (implements MID$, LEFT$, RIGHT$).
 */
BppStringRef str_mid(StringContext *ctx, BppStringRef ref, size_t start, size_t len);

/**
 * @brief Returns a direct pointer to the null-terminated string buffer.
 */
const char *str_data(BppStringRef ref);

/**
 * @brief Returns precomputed string length in bytes.
 */
size_t str_len(BppStringRef ref);
```

## 5. Architectural Invariants

- **Mandatory 2-Parameter `str_release()`**: Always call `str_release(vm_get_str(vm), str_ref)`.
- **Ownership Transfer Contract**: Statement handlers evaluating string expressions via `eval_expression()` acquire ownership of the string and MUST release it on both success and error paths.

## 6. Code Example: String Allocation and Slicing in C

```c
#include "runtime/strings.h"
#include <stdio.h>

void string_demo(StringContext *sc) {
    BppStringRef greeting = str_create(sc, "Hello, World!", 13);
    BppStringRef sub = str_mid(sc, greeting, 7, 5); /* Extracts "World" */

    printf("Extracted: %s (Len: %zu)\n", str_data(sub), str_len(sub));

    /* Release both handles */
    str_release(sc, greeting);
    str_release(sc, sub);
}
```
