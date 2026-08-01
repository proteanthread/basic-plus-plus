/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_strings.h
 * @brief Isolated string manager API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares functions for string creation, concatenation, slicing (MID$),
 *   reference counting, and memory cleanup.
 * - Why it exists: Strings are heavily mutated in BASIC. Isolating string allocations in a dedicated
 *   reference-counted heap prevents strings from corrupting variables, arrays, or the VM stack.
 * - Why it works this way: It uses opaque BppStringRef handles. Real string data is read-only
 *   and dynamically reference-counted, sharing buffers (Copy-on-Write) when possible to minimize allocations.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Slicing algorithms, string pooling behaviors.
 * - What cannot be changed: Opaque string reference handle definition (must remain BppString*).
 * - What to expect: Releasing a string reference decrements the ref count and automatically frees it when 0.
 * - What to do if something breaks: If memory leaks occur, check that str_release is matched with every
 *   str_create, str_concat, and variable assignment.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: String characters are 8-bit ASCII. Null-termination is maintained for C library compatibility.
 * - Portability concerns: None. C17 compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add string encoding conversions (UTF-8, EBCDIC) in the implementation.
 * - How to write external extensions: External plugins use these APIs to construct and manipulate strings
 *   without violating the isolated heap boundaries.
 */

#ifndef BPP_STRINGS_H
#define BPP_STRINGS_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "memory/memory.h"

/* Opaque String Manager Context */
typedef struct StringContext StringContext;

/**
 * @brief Initialize the string manager context.
 */
StringContext *str_init(MemoryContext *mem);

/**
 * @brief Shutdown the string manager and free all registered strings.
 */
void str_shutdown(StringContext *ctx);

/**
 * @brief Create a new string in the isolated heap.
 */
BppStringRef str_create(StringContext *ctx, const char *data, size_t length);

/**
 * @brief Concatenate two strings and return a new string.
 */
BppStringRef str_concat(StringContext *ctx, BppStringRef a, BppStringRef b);

/**
 * @brief Extract a substring (implements MID$).
 */
BppStringRef str_mid(StringContext *ctx, BppStringRef ref, size_t start, size_t len);

/**
 * @brief Retrieve raw text pointer from string reference.
 */
const char *str_data(BppStringRef ref);

/**
 * @brief Retrieve length of the string.
 */
size_t str_len(BppStringRef ref);

/**
 * @brief Increment reference count of a string.
 */
void str_add_ref(BppStringRef ref);

/**
 * @brief Decrement reference count of a string and free if count reaches 0.
 */
void str_release(StringContext *ctx, BppStringRef ref);

/**
 * @brief Run the compacting garbage collector to reclaim unused string slots.
 */
void str_gc(StringContext *ctx);

#endif /* BPP_STRINGS_H */
