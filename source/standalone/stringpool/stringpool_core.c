/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stringpool_core.c
 * Subsystem: Compacting String Pool Garbage Collector
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages text value strings and sweeps garbage.
 *
 * 2. WHAT TO EXPECT:
 *    Compacts active text variables shifting heap storage.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Buffer sizes constraints.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Compacting algorithm.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If collection crashes, verify variables ranges.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STRING POOL & GC
 * File: stringpool_core.c
 * ===================================================================== */

#include "stringpool_core.h"
#include <stdlib.h>
#include <string.h>

#define MAX_LIVE_STRINGS 1024

typedef struct {
    char **ptr;
    const char *old_data;
    int length;
} GcLiveString;

typedef struct {
    GcLiveString live[MAX_LIVE_STRINGS];
    int count;
    StringPool *pool;
} GcCompactContext;

int strpool_core_init(StringPool *pool, long size)
{
    if (pool == NULL) return -1;
    pool->base = (char *)malloc((size_t)size);
    if (pool->base == NULL) {
        pool->size = 0;
        pool->used = 0;
        return -1;
    }
    pool->size = size;
    pool->used = 0;
    return 0;
}

void strpool_core_free(StringPool *pool)
{
    if (pool == NULL) return;
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

char *strpool_core_alloc(StringPool *pool, int len)
{
    char *ptr;
    if (pool == NULL || pool->base == NULL || len < 0) return NULL;
    if (pool->used + len > pool->size) return NULL;
    ptr = pool->base + pool->used;
    pool->used += len;
    return ptr;
}

static void add_gc_ref(void *context, char **ptr, const char *data, int length)
{
    GcCompactContext *ctx = (GcCompactContext *)context;
    if (ctx->count >= MAX_LIVE_STRINGS) return;
    if (ptr == NULL || data == NULL || length <= 0) return;
    
    /* Verify data pointer resides inside the pool boundaries */
    if (data >= ctx->pool->base && data < ctx->pool->base + ctx->pool->size) {
        ctx->live[ctx->count].ptr = ptr;
        ctx->live[ctx->count].old_data = data;
        ctx->live[ctx->count].length = length;
        ctx->count++;
    }
}

int strpool_core_compact(StringPool *pool, GcMarkCallback mark_cb, void *context)
{
    long old_used;
    char *tmp;
    int ti, i;
    GcCompactContext ctx;

    if (pool == NULL || pool->base == NULL || mark_cb == NULL) return 0;
    old_used = pool->used;
    if (old_used == 0) return 0;

    ctx.count = 0;
    ctx.pool = pool;

    /* Invoke the client mark sweep callback */
    mark_cb(context, add_gc_ref, &ctx);

    if (ctx.count == 0) {
        pool->used = 0;
        return (int)old_used;
    }

    /* Allocate compaction buffer */
    tmp = (char *)malloc((size_t)old_used);
    if (tmp == NULL) return 0;

    ti = 0;
    for (i = 0; i < ctx.count; i++) {
        memcpy(tmp + ti, ctx.live[i].old_data, (size_t)ctx.live[i].length);
        ctx.live[i].old_data = tmp + ti;
        ti += ctx.live[i].length;
    }

    /* Sweep and copy back updating pointer references */
    pool->used = 0;
    for (i = 0; i < ctx.count; i++) {
        char *new_ptr = pool->base + pool->used;
        memcpy(new_ptr, ctx.live[i].old_data, (size_t)ctx.live[i].length);
        pool->used += ctx.live[i].length;
        *(ctx.live[i].ptr) = new_ptr;
    }

    free(tmp);
    return (int)(old_used - pool->used);
}
