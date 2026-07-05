/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: object_core.c
 * Subsystem: Standalone Object Pool Subsystem
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Handles static allocations of object instances and fields pool management.
 *
 * 2. WHAT TO EXPECT:
 *    Memory-efficient contiguous blocks allocation within a static array.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Initialization defaults, allocation checks.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Recursive release logic, double-allocation verification checks.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Asserts pool limits.
 * ===================================================================== */

#include "object_core.h"
#include <string.h>
#include <stdlib.h>

void object_pool_init(ObjectPoolContext *ctx)
{
    if (ctx == NULL) return;
    memset(ctx->object_pool, 0, sizeof(ctx->object_pool));
    for (int i = 0; i < MAX_OBJECT_INSTANCES; i++) {
        ctx->object_pool[i].class_id = -1;
        ctx->object_pool[i].field_start_idx = -1;
        ctx->object_pool[i].field_count = 0;
        ctx->object_pool[i].ref_count = 0;
    }
    memset(ctx->field_pool, 0, sizeof(ctx->field_pool));
    memset(ctx->field_pool_allocated, 0, sizeof(ctx->field_pool_allocated));
}

int object_pool_allocate_instance(ObjectPoolContext *ctx, int class_id, int field_count)
{
    if (ctx == NULL) return -1;
    
    // 1. Find free object slot
    int obj_id = -1;
    for (int i = 0; i < MAX_OBJECT_INSTANCES; i++) {
        if (ctx->object_pool[i].ref_count == 0) {
            obj_id = i;
            break;
        }
    }
    if (obj_id == -1) return -1; // pool full
    
    // 2. Find contiguous free block of fields
    int field_start = -1;
    if (field_count == 0) {
        field_start = 0;
    } else {
        for (int i = 0; i <= MAX_OBJECT_FIELDS - field_count; i++) {
            int ok = 1;
            for (int j = 0; j < field_count; j++) {
                if (ctx->field_pool_allocated[i + j]) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                field_start = i;
                break;
            }
        }
    }
    if (field_start == -1) return -2; // field pool full
    
    // 3. Mark fields as allocated and initialize them
    for (int j = 0; j < field_count; j++) {
        ctx->field_pool_allocated[field_start + j] = 1;
        ctx->field_pool[field_start + j].type = VAL_INTEGER;
        ctx->field_pool[field_start + j].v.ival = 0;
    }
    
    // 4. Populate instance
    ctx->object_pool[obj_id].class_id = class_id;
    ctx->object_pool[obj_id].field_start_idx = field_start;
    ctx->object_pool[obj_id].field_count = field_count;
    ctx->object_pool[obj_id].ref_count = 1;
    
    return obj_id;
}

void object_pool_retain(ObjectPoolContext *ctx, int obj_id)
{
    if (ctx == NULL) return;
    if (obj_id >= 0 && obj_id < MAX_OBJECT_INSTANCES) {
        ctx->object_pool[obj_id].ref_count++;
    }
}

void object_pool_release(ObjectPoolContext *ctx, int obj_id, void (*release_nested_cb)(void *user_data, BValue val), void *user_data)
{
    if (ctx == NULL) return;
    if (obj_id >= 0 && obj_id < MAX_OBJECT_INSTANCES) {
        ObjectInstance *obj = &ctx->object_pool[obj_id];
        if (obj->ref_count > 0) {
            obj->ref_count--;
            if (obj->ref_count == 0) {
                int start = obj->field_start_idx;
                int count = obj->field_count;
                if (start >= 0 && count > 0) {
                    for (int i = 0; i < count; i++) {
                        if (release_nested_cb) {
                            release_nested_cb(user_data, ctx->field_pool[start + i]);
                        }
                        ctx->field_pool_allocated[start + i] = 0;
                        ctx->field_pool[start + i].type = VAL_INTEGER;
                        ctx->field_pool[start + i].v.ival = 0;
                    }
                }
                obj->class_id = -1;
                obj->field_start_idx = -1;
                obj->field_count = 0;
            }
        }
    }
}
