/*
 * =====================================================================
 * BASIC++ Interpreter - stringpool.h
 * =====================================================================
 *
 * Simple bump-allocator string pool for BASIC string values.
 *
 * PURPOSE:
 *   Provides memory for string values during program execution.
 *   Strings are allocated from a contiguous block and never
 *   individually freed. The entire pool is reset at the start
 *   of each RUN, reclaiming all string memory.
 *
 * DESIGN RATIONALE:
 *   Most BASIC programs create a modest number of strings. A simple
 *   bump allocator avoids the complexity of garbage collection while
 *   being fast and predictable. The pool is sized to hold typical
 *   BASIC programs' string data (32K default).
 *
 *   If a program exhausts the string pool, ERR_SORRY is raised.
 *   The user can increase MAX_STRING_POOL in config.h.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#ifndef BASICPP_STRINGPOOL_H
#define BASICPP_STRINGPOOL_H

#include "config.h"

/* =====================================================================
 * StringPool - Bump allocator for string values
 * =====================================================================
 */
typedef struct StringPool {
    char *base;     /* start of allocated block */
    long  size;     /* total size in bytes */
    long  used;     /* current allocation watermark */
} StringPool;

/*
 * strpool_init - Allocate the string pool.
 *
 * Allocates MAX_STRING_POOL bytes via malloc.
 * Returns 0 on success, -1 on failure.
 */
int strpool_init(StringPool *pool);

/*
 * strpool_shutdown - Free the string pool.
 */
void strpool_shutdown(StringPool *pool);

/*
 * strpool_alloc - Allocate 'len' bytes from the string pool.
 *
 * Returns a pointer to the allocated space, or NULL if the
 * pool is exhausted. The returned memory is NOT zero-initialized.
 */
char *strpool_alloc(StringPool *pool, int len);

/*
 * strpool_reset - Reset the pool, reclaiming all allocations.
 *
 * Called at the start of each RUN. All previously allocated
 * strings become invalid.
 */
void strpool_reset(StringPool *pool);

/*
 * strpool_store - Copy a string into the pool.
 *
 * Convenience function: allocates 'len' bytes and copies
 * 'src' into the allocated space. Returns the pool pointer,
 * or NULL on failure.
 */
char *strpool_store(StringPool *pool, const char *src, int len);

#endif /* BASICPP_STRINGPOOL_H */
