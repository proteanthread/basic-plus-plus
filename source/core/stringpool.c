/*
 * ---
 * BASIC++ Interpreter - stringpool.c
 * ---
 *
 * Bump-allocator implementation for BASIC string values.
 *
 * ---
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

/* Runtime state pointer for auto-compact */
static void *s_compact_rt = NULL;

void strpool_set_runtime(StringPool *pool, void *rt)
{
 (void)pool;
 s_compact_rt = rt;
}

char *strpool_alloc(StringPool *pool, int len)
{
 char *ptr;
 if (len <= 0) return pool->base + pool->used; /* empty string */
 if (pool->used + (long)len > pool->size) {
 /* Try compaction before giving up */
 if (s_compact_rt != NULL) {
 int reclaimed = strpool_compact(pool, s_compact_rt);
 if (reclaimed > 0 &&
 pool->used + (long)len <= pool->size) {
 /* Compaction freed enough space */
 ptr = pool->base + pool->used;
 pool->used += (long)len;
 return ptr;
 }
 }
 return NULL; /* pool exhausted */
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

/*
 * strpool_compact - Mark-sweep string pool compaction.
 *
 * Walks all live string references in the RuntimeState,
 * copies their data to a temporary buffer, resets the pool,
 * and copies them back with updated pointers.
 *
 * Live string sources:
 *   1. string_vars[0..25] (A$-Z$)
 *   2. named_vars[0..named_count-1].value
 *   3. dim_elements[0..dim_elements_used-1]
 *   4. constants[0..const_count-1].value
 *   5. data_pool[0..data_count-1]
 *
 * Returns bytes reclaimed (old_used - new_used),
 * or 0 if compaction was not needed.
 */
#include "runtime.h"

/*
 * Helper: collect a BValue* string pointer for relocation.
 * We store the (pointer-to-BValue, old-data, old-len) triple.
 */
#define MAX_LIVE_STRINGS 4096
typedef struct {
 BValue *ref;   /* pointer to the BValue to update */
 char *old_data; /* original pool pointer */
 int length;
} LiveString;

int strpool_compact(StringPool *pool, void *runtime_state)
{
 RuntimeState *rt = (RuntimeState *)runtime_state;
 LiveString live[MAX_LIVE_STRINGS];
 int count = 0;
 long old_used;
 char *tmp;
 int ti, i;

 if (pool == NULL || rt == NULL) return 0;
 old_used = pool->used;
 if (old_used == 0) return 0;

 /* Phase 1: MARK - collect all live string refs */

 /* A$-Z$ */
 for (i = 0; i < MAX_STRING_VARS && count < MAX_LIVE_STRINGS; i++) {
 BValue *v = &rt->string_vars[i];
 if (bval_is_string(v) && v->v.sval.data != NULL &&
 v->v.sval.length > 0 &&
 v->v.sval.data >= pool->base &&
 v->v.sval.data < pool->base + pool->size) {
 live[count].ref = v;
 live[count].old_data = v->v.sval.data;
 live[count].length = v->v.sval.length;
 count++;
 }
 }

 /* Named variables */
 for (i = 0; i < rt->named_count && count < MAX_LIVE_STRINGS; i++) {
 BValue *v = &rt->named_vars[i].value;
 if (bval_is_string(v) && v->v.sval.data != NULL &&
 v->v.sval.length > 0 &&
 v->v.sval.data >= pool->base &&
 v->v.sval.data < pool->base + pool->size) {
 live[count].ref = v;
 live[count].old_data = v->v.sval.data;
 live[count].length = v->v.sval.length;
 count++;
 }
 }

 /* DIM array elements */
 for (i = 0; i < rt->dim_elements_used && count < MAX_LIVE_STRINGS; i++) {
 BValue *v = &rt->dim_elements[i];
 if (bval_is_string(v) && v->v.sval.data != NULL &&
 v->v.sval.length > 0 &&
 v->v.sval.data >= pool->base &&
 v->v.sval.data < pool->base + pool->size) {
 live[count].ref = v;
 live[count].old_data = v->v.sval.data;
 live[count].length = v->v.sval.length;
 count++;
 }
 }

 /* Constants */
 for (i = 0; i < rt->const_count && count < MAX_LIVE_STRINGS; i++) {
 BValue *v = &rt->constants[i].value;
 if (bval_is_string(v) && v->v.sval.data != NULL &&
 v->v.sval.length > 0 &&
 v->v.sval.data >= pool->base &&
 v->v.sval.data < pool->base + pool->size) {
 live[count].ref = v;
 live[count].old_data = v->v.sval.data;
 live[count].length = v->v.sval.length;
 count++;
 }
 }

 if (count == 0) {
 /* No live strings — just reset */
 pool->used = 0;
 return (int)old_used;
 }

 /* Phase 2: COPY - copy live data to temp buffer */
 tmp = (char *)malloc((size_t)old_used);
 if (tmp == NULL) return 0; /* can't compact */

 ti = 0;
 for (i = 0; i < count; i++) {
 memcpy(tmp + ti, live[i].old_data,
 (size_t)live[i].length);
 live[i].old_data = tmp + ti; /* point to temp copy */
 ti += live[i].length;
 }

 /* Phase 3: SWEEP - reset pool and copy back */
 pool->used = 0;
 for (i = 0; i < count; i++) {
 char *new_ptr = pool->base + pool->used;
 memcpy(new_ptr, live[i].old_data,
 (size_t)live[i].length);
 pool->used += live[i].length;
 /* Update the BValue pointer */
 live[i].ref->v.sval.data = new_ptr;
 }

 free(tmp);
 return (int)(old_used - pool->used);
}
