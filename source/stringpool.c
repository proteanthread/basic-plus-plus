/*
 * =====================================================================
 * BASIC++ Interpreter - stringpool.c
 * =====================================================================
 *
 * Bump-allocator implementation for BASIC string values.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdlib.h>
#include <string.h>
#include "stringpool.h"

int strpool_init(StringPool *pool)
{
    pool->base = (char *)malloc((size_t)MAX_STRING_POOL);
    if (pool->base == NULL) {
        pool->size = 0;
        pool->used = 0;
        return -1;
    }
    pool->size = MAX_STRING_POOL;
    pool->used = 0;
    return 0;
}

void strpool_shutdown(StringPool *pool)
{
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

char *strpool_alloc(StringPool *pool, int len)
{
    char *ptr;
    if (len <= 0) return pool->base + pool->used;  /* empty string */
    if (pool->used + (long)len > pool->size) {
        return NULL;  /* pool exhausted */
    }
    ptr = pool->base + pool->used;
    pool->used += (long)len;
    return ptr;
}

void strpool_reset(StringPool *pool)
{
    pool->used = 0;
}

char *strpool_store(StringPool *pool, const char *src, int len)
{
    char *ptr = strpool_alloc(pool, len);
    if (ptr != NULL && len > 0) {
        memcpy(ptr, src, (size_t)len);
    }
    return ptr;
}
