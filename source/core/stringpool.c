// ---
// BASIC++ Interpreter - stringpool.c
// ---
//
// Bump-allocator for BASIC string values with mark-sweep compaction.
//
// PURPOSE:
//   All BASIC string values (A$, named string variables, string
//   constants, string array elements) are stored in a single
//   contiguous memory pool. This file provides allocation,
//   storage, reset, and garbage-collection (compaction) for
//   that pool.
//
// HOW IT WORKS:
//   The string pool is a bump allocator:
//
//     Pool Layout:
//     +----------------------------------------------+
//     | "HELLO" | "WORLD" | "FOO" |   (free space)   |
//     +----------------------------------------------+
//     ^                           ^                   ^
//     base                       used                size
//
//   Allocation: advance 'used' by the requested length.
//   Storage:    allocate + memcpy the source string.
//   Reset:      set 'used' to 0 (all strings invalidated).
//
//   Strings are NOT individually freed. The pool grows
//   monotonically during execution. When the pool fills up,
//   the compactor (strpool_compact) runs a mark-sweep pass:
//
//   1. MARK:  Walk all live string references in RuntimeState
//             (A$-Z$, named vars, DIM elements, constants).
//   2. COPY:  Copy live strings to a temp buffer.
//   3. SWEEP: Reset pool, copy live strings back contiguously,
//             and update all BValue pointers to the new locations.
//
//   This reclaims space from dead (overwritten) strings without
//   pausing program execution for more than a few milliseconds.
//
// HOW TO EXTEND / CUSTOMIZE:
//   - To increase pool size: edit MAX_STRING_POOL in config.h.
//   - To add a new string source to compaction: add a new scan
//     loop in the MARK phase of strpool_compact() below.
//   - To disable compaction (simpler but wastes memory): remove
//     the auto-compact call in strpool_alloc().
//
// FINE-TUNING:
//   MAX_STRING_POOL sizes by build profile:
//     Modern:    16 MB  (generous for text-heavy programs)
//     FreeDOS:   32 KB  (fits in 512K conventional memory)
//     Embedded:   8 KB  (fits in ESP32 ~200 KB SRAM)
//
//   MAX_LIVE_STRINGS (4096) limits how many live string refs
//   the compactor can track in a single pass. If you have more
//   than 4096 live strings simultaneously, increase this constant.
//
//   To estimate string pool usage for a program:
//     total  (number of string assignments) x (average string length)
//   Compaction reclaims space from overwritten values.
//
// TROUBLESHOOTING:
//   - "SORRY" during string operations:
//     String pool exhausted. The program has allocated more string
//     data than MAX_STRING_POOL allows. Solutions:
//       1. Increase MAX_STRING_POOL in config.h.
//       2. Use shorter strings or fewer string variables.
//       3. The compactor runs automatically, but if most strings
//          are live (not overwritten), compaction won't help.
//
//   - Strings seem corrupted after compaction:
//     A string source was not included in the MARK phase.
//     Add a scan loop for the missing source in strpool_compact().
//
// PERFORMANCE:
//   - strpool_alloc():  O(1) -- pointer increment (no compaction)
//   - strpool_store():  O(n) -- memcpy of string data
//   - strpool_reset():  O(1) -- single assignment
//   - strpool_compact(): O(L + D) where L = live string count,
//     D = total live data bytes. Triggered automatically when
//     alloc fails, so cost is amortized over many allocations.
//
// MINIMALIZATION:
//   This file is CORE tier -- required for any build with strings.
//   For integer-only builds (PATB strict mode), the pool is still
//   allocated but mostly unused. To remove it entirely, you would
//   need to remove all string variable support from the runtime.
//
// DEPENDENCIES:
//   - stringpool.h (StringPool struct, function declarations)
//   - runtime.h    (RuntimeState for compaction -- live string scan)
//   - config.h     (MAX_STRING_POOL, MAX_STRING_VARS, etc.)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library (stdlib.h, string.h).
//   - No platform-specific code.
//   - Compiles cleanly on gcc -std=c17 and MSVC /std:c17.
//
// ---

#include <stdlib.h>
#include <string.h>
#include "stringpool.h"

// -----------------------------------------------------------------
// Pool Lifecycle
// -----------------------------------------------------------------

// strpool_init - Allocate the string pool.
//
// Allocates MAX_STRING_POOL bytes via malloc(). The pool starts
// empty (used = 0). All subsequent string storage goes here.
//
// Parameters:
//   pool - pointer to the StringPool to initialize
//
// Returns:
//   0 on success
//  -1 on allocation failure (pool fields set to safe defaults)
//
// Called by: runtime_init() in runtime.c during boot.
//
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

// strpool_shutdown - Free the string pool.
//
// After this call, ALL string pointers obtained from strpool_alloc()
// or strpool_store() are invalid. Called during interpreter shutdown.
//
// Safe to call multiple times (idempotent).
//
void strpool_shutdown(StringPool *pool)
{
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

// -----------------------------------------------------------------
// Auto-Compact Runtime Pointer
// -----------------------------------------------------------------

// The compactor needs access to RuntimeState to walk live string
// references, but stringpool.h can't include runtime.h (circular
// dependency). We store a void* pointer that gets cast in compact().
static void *s_compact_rt = NULL;

// strpool_set_runtime - Register the RuntimeState for auto-compact.
//
// Must be called once after runtime_init() so that strpool_alloc()
// can trigger automatic compaction when the pool fills up.
//
// Parameters:
//   pool - the string pool (unused, reserved for future multi-pool)
//   rt   - pointer to RuntimeState (stored as void* to avoid
//          circular header dependency)
//
void strpool_set_runtime(StringPool *pool, void *rt)
{
    (void)pool;  // reserved for future multi-pool support
    s_compact_rt = rt;
}

// -----------------------------------------------------------------
// Allocation & Storage
// -----------------------------------------------------------------

// strpool_alloc - Bump-allocate 'len' bytes from the string pool.
//
// Returns a pointer to 'len' contiguous bytes within the pool.
// The caller must copy string data into this region before the
// next allocation (the region is NOT zeroed).
//
// If the pool is full, attempts automatic compaction first.
// If compaction frees enough space, returns a valid pointer.
// If still insufficient, returns NULL (caller should raise SORRY).
//
// Special case: if len <= 0, returns a pointer to the current
// watermark (a valid empty-string location) without advancing it.
//
// Parameters:
//   pool - the string pool to allocate from
//   len  - number of bytes to allocate (must be > 0 for real data)
//
// Returns:
//   Pointer to the allocated region within the pool.
//   NULL if the pool is exhausted even after compaction.
//
char *strpool_alloc(StringPool *pool, int len)
{
    char *ptr;

    // Empty string: return current position without advancing
    if (len <= 0) return pool->base + pool->used;

    // Check if there's enough space
    if (pool->used + (long)len > pool->size) {
        // Try compaction before giving up.
        // This runs a mark-sweep pass over all live strings,
        // copying them to the front of the pool and reclaiming
        // space from dead (overwritten) strings.
        if (s_compact_rt != NULL) {
            int reclaimed = strpool_compact(pool, s_compact_rt);
            if (reclaimed > 0 &&
                pool->used + (long)len <= pool->size) {
                // Compaction freed enough space -- proceed
                ptr = pool->base + pool->used;
                pool->used += (long)len;
                return ptr;
            }
        }
        return NULL;  // pool exhausted, even after compaction
    }

    // Normal allocation: advance watermark
    ptr = pool->base + pool->used;
    pool->used += (long)len;
    return ptr;
}

// strpool_reset - Reset the string pool to empty.
//
// All previously allocated strings become invalid. Called by
// runtime_reset() on RUN and NEW to start with a clean string pool.
//
// The memory is NOT zeroed (for performance). Since all strings
// are re-allocated on first use, stale data is never accessed.
//
void strpool_reset(StringPool *pool)
{
    pool->used = 0;
}

// strpool_store - Allocate and copy a string into the pool.
//
// Convenience function that combines strpool_alloc() + memcpy().
// This is the most common way to add a string to the pool.
//
// Parameters:
//   pool - the string pool
//   src  - source string data to copy (NOT null-terminated in pool)
//   len  - number of bytes to copy
//
// Returns:
//   Pointer to the stored copy within the pool.
//   NULL if the pool is exhausted.
//
// Note: Strings in the pool are NOT null-terminated. The BValue
// struct stores (data_pointer, length) pairs for zero-copy access.
// This saves 1 byte per string and allows embedded NUL characters.
//
char *strpool_store(StringPool *pool, const char *src, int len)
{
    char *ptr = strpool_alloc(pool, len);
    if (ptr != NULL && len > 0) {
        memcpy(ptr, src, (size_t)len);
    }
    return ptr;
}

// -----------------------------------------------------------------
// Mark-Sweep Compaction (Garbage Collection)
// -----------------------------------------------------------------
//
// strpool_compact - Reclaim dead space in the string pool.
//
// When strings are overwritten (e.g., A$ = "OLD" then A$ = "NEW"),
// the old string data remains in the pool as dead space. Over time,
// this wastes memory. Compaction reclaims it by copying only live
// strings to the front of the pool.
//
// Algorithm:
//   Phase 1 -- MARK:
//     Walk all live string references in RuntimeState:
//       - string_vars[0..25]  (A$-Z$)
//       - named_vars[0..named_count-1].value
//       - dim_elements[0..dim_elements_used-1]
//       - constants[0..const_count-1].value
//     For each live string, record its BValue pointer, data pointer,
//     and length in the live[] array.
//
//   Phase 2 -- COPY:
//     Allocate a temporary buffer and copy all live string data
//     into it, preserving order.
//
//   Phase 3 -- SWEEP:
//     Reset the pool watermark to 0. Copy live strings back from
//     the temp buffer into the pool contiguously. Update each
//     BValue's data pointer to the new location.
//
// Parameters:
//   pool          - the string pool to compact
//   runtime_state - pointer to RuntimeState (cast from void*)
//
// Returns:
//   Number of bytes reclaimed (old_used - new_used).
//   0 if compaction was not needed or could not run (malloc fail).
//
// How to customize:
//   To add new string sources (e.g., from a new module's state):
//   1. Add a new scan loop in Phase 1 below.
//   2. Follow the same pattern: check bval_is_string(), verify
//      the data pointer is within the pool, and record in live[].
//   3. Increase MAX_LIVE_STRINGS if needed.
//
// Performance:
//   O(L + D) where L = number of live strings, D = total data bytes.
//   The temporary malloc is freed immediately after. Compaction
//   runs only when strpool_alloc() fails, so it's infrequent.
//
// Limitations:
//   MAX_LIVE_STRINGS (4096) limits simultaneous live string refs.
//   If you exceed this, some dead strings won't be reclaimed.
//   The data_pool[] (DATA/READ values) is not compacted to avoid
//   invalidating the data pointer during program execution.
//

#include "runtime.h"

// Helper struct: tracks a live string reference for relocation.
// We store the BValue pointer (so we can update it), the old
// data pointer (so we can find the data), and the length.
#define MAX_LIVE_STRINGS 4096
typedef struct {
    BValue *ref;       // pointer to the BValue to update
    char   *old_data;  // original pool pointer (before compaction)
    int     length;    // string length in bytes
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

    // -------------------------------------------------------
    // Phase 1: MARK -- collect all live string references
    // -------------------------------------------------------
    // For each string source, check if the string data lives
    // within this pool (not a static string from source code).

    // Scan A$-Z$ string variables
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

    // Scan named variables (multi-character identifiers)
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

    // Scan DIM array elements (string arrays like A$(10))
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

    // Scan CONST values (CONST FOO$ = "BAR")
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
        // No live strings -- just reset the pool entirely
        pool->used = 0;
        return (int)old_used;
    }

    // -------------------------------------------------------
    // Phase 2: COPY -- copy live data to a temporary buffer
    // -------------------------------------------------------
    tmp = (char *)malloc((size_t)old_used);
    if (tmp == NULL) return 0;  // can't compact (malloc failed)

    ti = 0;
    for (i = 0; i < count; i++) {
        memcpy(tmp + ti, live[i].old_data,
            (size_t)live[i].length);
        live[i].old_data = tmp + ti;  // update to point into temp
        ti += live[i].length;
    }

    // -------------------------------------------------------
    // Phase 3: SWEEP -- reset pool and copy live data back
    // -------------------------------------------------------
    pool->used = 0;
    for (i = 0; i < count; i++) {
        char *new_ptr = pool->base + pool->used;
        memcpy(new_ptr, live[i].old_data,
            (size_t)live[i].length);
        pool->used += live[i].length;
        // Update the BValue to point to the new pool location
        live[i].ref->v.sval.data = new_ptr;
    }

    free(tmp);
    return (int)(old_used - pool->used);
}
