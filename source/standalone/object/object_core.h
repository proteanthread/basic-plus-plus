/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: object_core.h
 * Subsystem: Standalone Object Pool Subsystem
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Manages the lifecycle, static fields allocation, retention and recursive
 *    release of pool-bounded objects.
 *
 * 2. WHAT TO EXPECT:
 *    Strictly pool-bounded. No malloc/free dynamic allocations.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Pool limits, lifecycle callbacks.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Reference counting model, static contiguous fields allocation logic.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Raises SORRY exceptions if pool limits are exceeded.
 * ===================================================================== */

#ifndef OBJECT_CORE_H
#define OBJECT_CORE_H

#include "../../value.h"

#define MAX_CLASSES          32    /* Maximum active class definitions */
#define MAX_OBJECT_INSTANCES 128   /* Maximum concurrently active objects */
#define MAX_OBJECT_FIELDS    1024  /* Total field slots across all active objects */

typedef struct {
    int class_id;        /* Index into class definitions table */
    int field_start_idx; /* Offset into the global Object Fields Pool */
    int field_count;     /* Number of fields in this object instance */
    int ref_count;       /* Simple reference tracker for reuse validation */
} ObjectInstance;

/* Standalone Object Pool Context */
typedef struct {
    ObjectInstance object_pool[MAX_OBJECT_INSTANCES];
    BValue field_pool[MAX_OBJECT_FIELDS];
    char field_pool_allocated[MAX_OBJECT_FIELDS];
} ObjectPoolContext;

void object_pool_init(ObjectPoolContext *ctx);
int object_pool_allocate_instance(ObjectPoolContext *ctx, int class_id, int field_count);
void object_pool_retain(ObjectPoolContext *ctx, int obj_id);
void object_pool_release(ObjectPoolContext *ctx, int obj_id, void (*release_nested_cb)(void *user_data, BValue val), void *user_data);

#endif /* OBJECT_CORE_H */
