/*
 * =====================================================================
 * BASIC++ Interpreter - memory.c
 * =====================================================================
 *
 * Implementation of the memory management subsystem.
 *
 * DESIGN RATIONALE:
 *   The interpreter uses explicit memory pools rather than calling
 *   malloc()/free() for individual objects. This approach:
 *
 *   1. Prevents fragmentation - bump allocation is contiguous.
 *   2. Makes memory usage predictable and bounded.
 *   3. Simplifies cleanup - free the pool, done.
 *   4. Works well on memory-constrained systems (FreeDOS).
 *   5. Allows SIZE to report accurate free-space counts.
 *
 *   The program store uses a sorted array rather than a linked list
 *   because:
 *   - Binary search for GOTO targets is O(log n).
 *   - Sequential iteration for RUN is cache-friendly.
 *   - Insertion/deletion with memmove is O(n) but n <= 4096,
 *     and program editing is infrequent compared to execution.
 *
 * ANSI C89/C90 COMPLIANT
 * =====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "errors.h"

/* =====================================================================
 * Pool Management
 * =====================================================================
 */

/*
 * init_pool - Internal helper to allocate and initialize one pool.
 *
 * Allocates 'size' bytes via malloc(). Zeroes the watermark.
 * Returns 0 on success, -1 on allocation failure.
 */
static int init_pool(MemoryPool *pool, long size)
{
    pool->base = (char *)malloc((size_t)size);
    if (pool->base == NULL) {
        pool->size = 0;
        pool->used = 0;
        return -1;
    }
    pool->size = size;
    pool->used = 0;

    /*
     * Zero the pool memory. This ensures variables and array
     * elements start at zero, matching BASIC convention.
     */
    memset(pool->base, 0, (size_t)size);
    return 0;
}

/*
 * free_pool - Internal helper to free one pool.
 *
 * Safe to call on an uninitialized or already-freed pool
 * (checks for NULL).
 */
static void free_pool(MemoryPool *pool)
{
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

/*
 * mem_init - Allocate all memory pools and the program store.
 *
 * Allocation order:
 *   1. Variable pool (for A-Z variables and @() array)
 *   2. Scratch pool (for temporary tokens and parse buffers)
 *   3. Program line array (for sorted ProgramLine storage)
 *
 * If any allocation fails, all previously allocated pools are freed
 * and -1 is returned. The caller should report ERR_SORRY.
 */
int mem_init(MemorySystem *mem)
{
    /* Initialize all fields to safe defaults */
    mem->variable.base = NULL;
    mem->variable.size = 0;
    mem->variable.used = 0;
    mem->scratch.base = NULL;
    mem->scratch.size = 0;
    mem->scratch.used = 0;
    mem->program.lines = NULL;
    mem->program.count = 0;
    mem->program.capacity = 0;

    /* Allocate variable pool */
    if (init_pool(&mem->variable, VARIABLE_MEMORY_SIZE) != 0) {
        return -1;
    }

    /* Allocate scratch pool */
    if (init_pool(&mem->scratch, SCRATCH_MEMORY_SIZE) != 0) {
        free_pool(&mem->variable);
        return -1;
    }

    /*
     * Allocate program line array.
     *
     * We use a separate malloc for the line array rather than
     * carving it from a pool because ProgramLine structs are
     * relatively large (260+ bytes each) and the array needs
     * to be contiguous for memmove operations during insert/delete.
     */
    mem->program.lines = (ProgramLine *)malloc(
        (size_t)MAX_PROGRAM_LINES * sizeof(ProgramLine)
    );
    if (mem->program.lines == NULL) {
        free_pool(&mem->scratch);
        free_pool(&mem->variable);
        return -1;
    }
    mem->program.count = 0;
    mem->program.capacity = MAX_PROGRAM_LINES;

    /* Zero the line array for clean initial state */
    memset(mem->program.lines, 0,
           (size_t)MAX_PROGRAM_LINES * sizeof(ProgramLine));

    return 0;
}

/*
 * mem_shutdown - Free all pools and the program store.
 */
void mem_shutdown(MemorySystem *mem)
{
    free_pool(&mem->variable);
    free_pool(&mem->scratch);

    if (mem->program.lines != NULL) {
        free(mem->program.lines);
        mem->program.lines = NULL;
    }
    mem->program.count = 0;
    mem->program.capacity = 0;
}

/*
 * mem_pool_alloc - Bump-allocate bytes from a pool.
 *
 * Returns a pointer to the allocated region within the pool.
 * Returns NULL if there is insufficient space. The caller is
 * responsible for checking the return value and raising ERR_SORRY
 * if needed.
 *
 * No alignment guarantee beyond char alignment. For long-aligned
 * access, the caller should request a size that is a multiple of
 * sizeof(long).
 */
void *mem_pool_alloc(MemoryPool *pool, long nbytes)
{
    char *ptr;

    if (nbytes <= 0) {
        return NULL;
    }

    if (pool->used + nbytes > pool->size) {
        return NULL;  /* insufficient space */
    }

    ptr = pool->base + pool->used;
    pool->used += nbytes;
    return (void *)ptr;
}

/*
 * mem_pool_reset - Reset a pool's watermark.
 *
 * All previous allocations from this pool become invalid.
 * The memory itself is not zeroed (for performance).
 * Primarily used for the scratch pool between statements.
 */
void mem_pool_reset(MemoryPool *pool)
{
    pool->used = 0;
}

/*
 * mem_pool_available - Return remaining bytes in a pool.
 *
 * Used for the SIZE function and capacity checks.
 */
long mem_pool_available(MemoryPool *pool)
{
    return pool->size - pool->used;
}

/* =====================================================================
 * Program Store Operations
 * =====================================================================
 */

/*
 * find_insert_pos - Binary search for insertion position.
 *
 * Returns the index where a line with the given number should be
 * inserted to maintain sorted order. If a line with that number
 * already exists, returns its index (caller checks for replacement).
 *
 * This is a standard binary search on a sorted array of line numbers.
 */
static int find_insert_pos(ProgramStore *store, int line_number)
{
    int low = 0;
    int high = store->count - 1;
    int mid;

    while (low <= high) {
        mid = low + (high - low) / 2;
        if (store->lines[mid].line_number == line_number) {
            return mid;  /* exact match */
        } else if (store->lines[mid].line_number < line_number) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return low;  /* insertion point */
}

/*
 * program_insert - Insert or replace a program line.
 *
 * If a line with the given number exists, its text is replaced.
 * If not, a new entry is inserted at the correct sorted position
 * using memmove to shift subsequent lines.
 *
 * The full_text parameter should be the complete line as entered
 * by the user (e.g., "10 PRINT \"HELLO\"").
 *
 * Returns 0 on success, -1 if the store is full.
 */
int program_insert(ProgramStore *store, int line_number,
                   const char *full_text)
{
    int pos;
    int i;

    pos = find_insert_pos(store, line_number);

    /* Check if this is a replacement of an existing line */
    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        /* Replace existing line text */
        strncpy(store->lines[pos].text, full_text, MAX_LINE_LENGTH);
        store->lines[pos].text[MAX_LINE_LENGTH] = '\0';
        return 0;
    }

    /* Inserting a new line - check capacity */
    if (store->count >= store->capacity) {
        error_raise(ERR_SORRY, 0);
        return -1;
    }

    /*
     * Shift lines from pos..count-1 up by one position.
     * We use a manual loop instead of memmove for clarity and
     * because we're moving structs (memmove would work too, but
     * this is explicit about what's happening).
     */
    for (i = store->count; i > pos; i--) {
        store->lines[i] = store->lines[i - 1];
    }

    /* Insert the new line */
    store->lines[pos].line_number = line_number;
    strncpy(store->lines[pos].text, full_text, MAX_LINE_LENGTH);
    store->lines[pos].text[MAX_LINE_LENGTH] = '\0';
    store->count++;

    return 0;
}

/*
 * program_delete - Delete a line by number.
 *
 * Finds the line via binary search and removes it by shifting
 * subsequent lines down.
 *
 * Returns 0 on success, -1 if the line was not found.
 */
int program_delete(ProgramStore *store, int line_number)
{
    int pos;
    int i;

    pos = find_insert_pos(store, line_number);

    /* Verify we found an exact match */
    if (pos >= store->count ||
        store->lines[pos].line_number != line_number) {
        return -1;  /* line not found (not an error in BASIC) */
    }

    /* Shift lines down to fill the gap */
    for (i = pos; i < store->count - 1; i++) {
        store->lines[i] = store->lines[i + 1];
    }

    store->count--;
    return 0;
}

/*
 * program_find - Find a line by exact number.
 *
 * Returns the index into the lines array, or -1 if not found.
 */
int program_find(ProgramStore *store, int line_number)
{
    int pos;

    pos = find_insert_pos(store, line_number);

    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        return pos;
    }

    return -1;
}

/*
 * program_find_next - Find first line with number >= target.
 *
 * Used by GOTO to locate the target line. If the exact line exists,
 * returns its index. Otherwise returns the index of the next higher
 * line. Returns -1 if no line has a number >= target.
 *
 * This is important for PATB behavior: GOTO to a nonexistent line
 * number should raise HOW?, but this function supports the lookup
 * needed to determine that.
 */
int program_find_next(ProgramStore *store, int line_number)
{
    int pos;

    pos = find_insert_pos(store, line_number);

    if (pos < store->count) {
        return pos;
    }

    return -1;  /* no line at or after this number */
}

/*
 * program_clear - Remove all stored lines (NEW command).
 *
 * Resets the count to zero. The array memory remains allocated
 * for reuse.
 */
void program_clear(ProgramStore *store)
{
    store->count = 0;
}

/*
 * program_list - Print program lines to stdout (LIST command).
 *
 * Lists all lines with line numbers in the range [from, to].
 * If from <= 0, starts from the first stored line.
 * If to <= 0, lists through the last stored line.
 *
 * Output format: each line is printed exactly as stored, followed
 * by a newline. This preserves the user's original formatting.
 */
void program_list(ProgramStore *store, int from, int to)
{
    int i;

    for (i = 0; i < store->count; i++) {
        if (from > 0 && store->lines[i].line_number < from) {
            continue;
        }
        if (to > 0 && store->lines[i].line_number > to) {
            break;  /* lines are sorted, so no more matches */
        }
        printf("%s\n", store->lines[i].text);
    }
}
