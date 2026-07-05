/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stringpool_core.h
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
 * File: stringpool_core.h
 * ===================================================================== */

#ifndef STANDALONE_STRINGPOOL_CORE_H
#define STANDALONE_STRINGPOOL_CORE_H

typedef struct StringPool {
    char *base;
    long size;
    long used;
} StringPool;

/* Initialize and free the string pool */
int strpool_core_init(StringPool *pool, long size);
void strpool_core_free(StringPool *pool);

/* Allocate a string of length inside the pool */
char *strpool_core_alloc(StringPool *pool, int length);

/* Callback to collect references during the mark phase */
typedef void (*GcMarkCallback)(void *context, void (*add_ref)(void *ref_context, char **ptr, const char *data, int length), void *ref_context);

/* Compact string pool, calling mark_cb to register live references and updating pointer targets */
int strpool_core_compact(StringPool *pool, GcMarkCallback mark_cb, void *context);

#endif /* STANDALONE_STRINGPOOL_CORE_H */
