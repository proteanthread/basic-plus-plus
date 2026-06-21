/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memory.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - memory.c
// ---
//
// Implementation of the memory management subsystem.
//
// PURPOSE:
//   Provides three memory pools (variable, scratch, program store)
//   that the interpreter uses for all runtime storage. No other
//   module calls malloc() directly -- all memory flows through here.
//
// HOW IT WORKS:
//   The interpreter uses bump-allocated memory pools instead of
//   individual malloc()/free() calls. Each pool is a single
//   contiguous block of memory with a watermark pointer:
//
//     Pool Layout:
//     +------------------------------------------+
//     | base                         | used -->   |
//     +------------------------------------------+
//     ^                              ^            ^
//     0                            used          size
//     (allocated region)            (free region)
//
//   Allocation: advance 'used' by the requested size.
//   Reset:      set 'used' back to 0 (scratch pool only).
//   Free:       free(base) at shutdown.
//
//   The program store uses a sorted array of ProgramLine structs.
//   Lines are kept sorted by line number at all times. Binary
//   search gives O(log n) GOTO lookup. Insertion/deletion use
//   memmove to shift entries, which is O(n) but n <= 65536 and
//   edits are infrequent compared to execution.
//
// HOW TO EXTEND / CUSTOMIZE:
//
// ===================================================================
// ADDING A NEW MEMORY POOL -- Step-by-Step Guide
// ===================================================================
//
//   The memory system uses generic MemoryPool structs for all pools.
//   Adding a new pool (for a stack, graphics buffer, sound buffer,
//   or any other purpose) is a 4-step process:
//
//   STEP 1: Add the pool to MemorySystem (memory.h)
//   -------------------------------------------------
//   Open memory.h and add a new MemoryPool field to the struct:
//
//     typedef struct MemorySystem {
//         MemoryPool variable;
//         MemoryPool scratch;
//         MemoryPool stack;      // <-- NEW: GOSUB/FOR call stack
//         ProgramStore program;
//     } MemorySystem;
//
//   STEP 2: Add the size constant to config.h
//   ------------------------------------------
//   Open config.h and add a #define for each build profile:
//
//     // In the MODERN BUILD section:
//     #define STACK_POOL_SIZE    262144L   // 256 KB
//
//     // In the BPP_FREEDOS section:
//     #define STACK_POOL_SIZE     8192L    //   8 KB
//
//     // In the BPP_EMBEDDED section:
//     #define STACK_POOL_SIZE     2048L    //   2 KB
//
//   STEP 3: Initialize and free the pool (memory.c -- this file)
//   ------------------------------------------------------------
//   In mem_init(), add init_pool() AFTER the existing pools:
//
//     // Allocate stack pool
//     if (init_pool(&mem->stack, STACK_POOL_SIZE) != 0) {
//         free_pool(&mem->scratch);
//         free_pool(&mem->variable);
//         return -1;
//     }
//
//   In mem_shutdown(), add free_pool() in REVERSE order:
//
//     free_pool(&mem->stack);    // <-- NEW
//     free_pool(&mem->scratch);
//     free_pool(&mem->variable);
//
//   STEP 4: Allocate from the pool in your module
//   ----------------------------------------------
//   Use the generic pool allocator from any module:
//
//     void *buf = mem_pool_alloc(&mem->stack, sizeof(CallFrame));
//     if (!buf) { error_raise(ERR_SORRY, line_num); return; }
//
//   Use mem_pool_reset() to reclaim all space in the pool:
//
//     mem_pool_reset(&mem->stack);  // reclaim after RUN completes
//
//   Use mem_pool_available() to check remaining space:
//
//     long free_bytes = mem_pool_available(&mem->stack);
//
// ===================================================================
// EXAMPLE POOLS -- Common Use Cases
// ===================================================================
//
//   Graphics Framebuffer Pool:
//   -------------------------
//   Purpose: Off-screen pixel buffer for DRAW, LINE, CIRCLE, etc.
//   Sizing:  GFX_WIDTH x GFX_HEIGHT x bytes_per_pixel
//
//     // config.h:
//     #define GFX_POOL_SIZE  (GFX_WIDTH * GFX_HEIGHT * 4L)
//     // 640x480x4 = 1,228,800 bytes (~1.2 MB) for RGBA
//     // 320x200x1 =    64,000 bytes (~62 KB) for indexed color
//     // 160x100x1 =    16,000 bytes (~16 KB) for FreeDOS
//
//     // memory.h -- add to MemorySystem:
//     MemoryPool gfx;
//
//     // Allocate in your graphics module:
//     unsigned char *fb = mem_pool_alloc(&mem->gfx, width * height);
//
//   Sound Buffer Pool:
//   -----------------
//   Purpose: PCM sample storage for SOUND, PLAY, BEEP commands.
//   Sizing:  sample_rate x duration_seconds x bytes_per_sample
//
//     // config.h:
//     #define SOUND_POOL_SIZE  176400L
//     // 44100 Hz x 2 seconds x 2 bytes (16-bit) = 176,400 bytes
//     // For FreeDOS (PC speaker): 8192L is sufficient
//     // For embedded: omit entirely (no sound support)
//
//     // memory.h -- add to MemorySystem:
//     MemoryPool sound;
//
//   Call Stack Pool:
//   ---------------
//   Purpose: Separate pool for GOSUB return addresses and FOR
//            loop frames, instead of using the C stack.
//   Sizing:  max_depth x sizeof(CallFrame)
//
//     // config.h:
//     #define STACK_POOL_SIZE  (MAX_STACK_DEPTH * 32L)
//     // 1024 x 32 = 32,768 bytes (32 KB) for modern
//     //   64 x 32 =  2,048 bytes ( 2 KB) for FreeDOS
//
//   Network I/O Buffer Pool:
//   -----------------------
//   Purpose: Buffer space for TCP/UDP socket operations.
//
//     // config.h:
//     #define NET_POOL_SIZE  65536L   // 64 KB
//     // Only needed if networking module is compiled in.
//     // Set to 0 or omit for BPP_FREEDOS/BPP_EMBEDDED.
//
//   User Type Pool:
//   --------------
//   Purpose: Storage for user-defined TYPE structures.
//
//     // config.h:
//     #define TYPE_POOL_SIZE  131072L  // 128 KB
//
// ===================================================================
// IMPORTANT NOTES FOR POOL DESIGN
// ===================================================================
//
//   Bump allocation only:
//     Pools do NOT support individual deallocation. You can only
//     allocate forward (advance watermark) or reset everything
//     (set watermark to 0). If you need per-object free, use a
//     different strategy (e.g., a free list within the pool).
//
//   Alignment:
//     mem_pool_alloc() returns char-aligned pointers. If your
//     data needs stricter alignment (e.g., 8-byte for doubles),
//     round up nbytes to the next alignment boundary:
//       long aligned = (nbytes + 7L) & ~7L;
//
//   Thread safety:
//     Pools are NOT thread-safe. If you add multi-threading,
//     protect pool access with a mutex.
//
//   Pool order in mem_init():
//     Always allocate CRITICAL pools first (variable, scratch,
//     program) and OPTIONAL pools last (gfx, sound, net). On
//     allocation failure, free in REVERSE order.
//
//   - To change existing pool sizes: edit config.h constants
//     (VARIABLE_MEMORY_SIZE, SCRATCH_MEMORY_SIZE, etc.). See
//     config.h for platform-specific sizing guides and memory.c
//     "DYNAMIC MEMORY SIZING GUIDE" for per-RAM-tier configs.
//
// FINE-TUNING:
//   Pool sizes are set in config.h and vary by build profile:
//
//   Constant               Modern    FreeDOS   Embedded
//   --------               ------    -------   --------
//   VARIABLE_MEMORY_SIZE   1 MB      16 KB     4 KB
//   SCRATCH_MEMORY_SIZE    512 KB    16 KB     2 KB
//   MAX_PROGRAM_LINES      65536     1024      256
//   MAX_STRING_POOL        16 MB     32 KB     8 KB
//   MAX_ARRAY_ELEMENTS     4M        4096      1024
//
//   What each pool controls:
//     VARIABLE_MEMORY_SIZE -- Storage for all numeric variables (A-Z,
//       named variables, and their metadata). Each named variable
//       uses ~40 bytes. 1 MB supports ~25,000 named variables.
//     SCRATCH_MEMORY_SIZE -- Temporary space for expression evaluation
//       and intermediate string operations. Only needed during
//       execution; reset between statements. 512 KB handles
//       deeply nested expressions (100+ levels).
//     MAX_PROGRAM_LINES -- Maximum number of BASIC source lines.
//       Each slot costs ~260 bytes whether used or not. 65536
//       lines costs ~17 MB in slot overhead alone.
//     MAX_STRING_POOL -- String storage pool (managed by stringpool.c).
//       All string variables, string literals, and concatenation
//       results live here. 16 MB supports ~500K short strings.
//     MAX_ARRAY_ELEMENTS -- Total elements across ALL @() arrays
//       and DIM arrays. Each element is 16 bytes (BValue).
//       4M elements = 64 MB of array storage.
//
//   To estimate total BASIC++ memory footprint:
//     program store   MAX_PROGRAM_LINES x 260 bytes
//     variable pool  = VARIABLE_MEMORY_SIZE
//     scratch pool   = SCRATCH_MEMORY_SIZE
//     string pool    = MAX_STRING_POOL
//     array pool     = MAX_ARRAY_ELEMENTS x 16 bytes
//     runtime state   30 KB (fixed overhead)
//     code + stacks   5 MB (executable + C call stack)
//     --------------------------------------------------
//     Total  sum of the above
//
// ===================================================================
// DYNAMIC MEMORY SIZING GUIDE -- Tuning for Your System
// ===================================================================
//
//   BASIC++ uses FIXED-SIZE pools allocated at startup. The pools
//   do NOT grow dynamically -- they are allocated once via malloc()
//   during boot (Phase 1) and remain that size until shutdown.
//   This means you need to choose pool sizes BEFORE compiling.
//
//   The defaults (Modern profile) use ~90 MB total, which is fine
//   for any system with 4 GB+ RAM. But if you want to maximize
//   BASIC++ for your hardware, here are recommended configurations:
//
//   --- 4 GB RAM (slim laptop, VM, Raspberry Pi 4) ---------------
//   Available after OS: ~2.5 GB. Keep BASIC++ under 100 MB.
//
//     #define PROGRAM_MEMORY_SIZE    4194304L   //   4 MB
//     #define VARIABLE_MEMORY_SIZE    524288L   // 512 KB
//     #define SCRATCH_MEMORY_SIZE     262144L   // 256 KB
//     #define MAX_PROGRAM_LINES       32768     //  32K lines
//     #define MAX_STRING_POOL        4194304L   //   4 MB
//     #define MAX_ARRAY_ELEMENTS     1048576    //   1M elements
//     #define MAX_NAMED_VARS          2048
//     #define MAX_DATA_ITEMS         32768
//     // Estimated total: ~30 MB  (leaves >2 GB for OS)
//
//   --- 8 GB RAM (standard desktop/laptop) -----------------------
//   Available after OS: ~5.5 GB. BASIC++ can use 200-500 MB.
//
//     #define PROGRAM_MEMORY_SIZE    8388608L   //   8 MB
//     #define VARIABLE_MEMORY_SIZE   1048576L   //   1 MB
//     #define SCRATCH_MEMORY_SIZE     524288L   // 512 KB
//     #define MAX_PROGRAM_LINES       65536     //  64K lines
//     #define MAX_STRING_POOL       16777216L   //  16 MB
//     #define MAX_ARRAY_ELEMENTS     4194304    //   4M elements
//     #define MAX_NAMED_VARS          4096
//     #define MAX_DATA_ITEMS         65536
//     // Estimated total: ~90 MB  (THIS IS THE DEFAULT)
//
//   --- 16 GB RAM (developer workstation) ------------------------
//   Available after OS: ~12 GB. BASIC++ can use 500 MB-2 GB.
//
//     #define PROGRAM_MEMORY_SIZE   16777216L   //  16 MB
//     #define VARIABLE_MEMORY_SIZE   4194304L   //   4 MB
//     #define SCRATCH_MEMORY_SIZE    1048576L   //   1 MB
//     #define MAX_PROGRAM_LINES      131072     // 128K lines
//     #define MAX_STRING_POOL       67108864L   //  64 MB
//     #define MAX_ARRAY_ELEMENTS     8388608    //   8M elements
//     #define MAX_NAMED_VARS          8192
//     #define MAX_DATA_ITEMS        131072
//     // Estimated total: ~350 MB
//
//   --- 32 GB RAM (high-end workstation) -------------------------
//   Available after OS: ~26 GB. BASIC++ can use 2-8 GB.
//
//     #define PROGRAM_MEMORY_SIZE   33554432L   //  32 MB
//     #define VARIABLE_MEMORY_SIZE  16777216L   //  16 MB
//     #define SCRATCH_MEMORY_SIZE    4194304L   //   4 MB
//     #define MAX_PROGRAM_LINES      262144     // 256K lines
//     #define MAX_STRING_POOL      268435456L   // 256 MB
//     #define MAX_ARRAY_ELEMENTS    33554432     //  32M elements
//     #define MAX_NAMED_VARS         16384
//     #define MAX_DATA_ITEMS        262144
//     // Estimated total: ~1 GB
//
//   --- 64 GB RAM (data science / server) ------------------------
//   Available after OS: ~56 GB. BASIC++ can use 4-16 GB.
//
//     #define PROGRAM_MEMORY_SIZE   67108864L   //  64 MB
//     #define VARIABLE_MEMORY_SIZE  33554432L   //  32 MB
//     #define SCRATCH_MEMORY_SIZE   16777216L   //  16 MB
//     #define MAX_PROGRAM_LINES      524288     // 512K lines
//     #define MAX_STRING_POOL      536870912L   // 512 MB
//     #define MAX_ARRAY_ELEMENTS    67108864     //  64M elements
//     #define MAX_NAMED_VARS         32768
//     #define MAX_DATA_ITEMS        524288
//     // Estimated total: ~2 GB
//
//   --- 128 GB+ RAM (high-memory server) -------------------------
//   Available after OS: ~120 GB. BASIC++ can use 8-32 GB.
//
//     #define PROGRAM_MEMORY_SIZE  134217728L   // 128 MB
//     #define VARIABLE_MEMORY_SIZE  67108864L   //  64 MB
//     #define SCRATCH_MEMORY_SIZE   33554432L   //  32 MB
//     #define MAX_PROGRAM_LINES     1048576     //   1M lines
//     #define MAX_STRING_POOL     1073741824L   //   1 GB
//     #define MAX_ARRAY_ELEMENTS   134217728     // 128M elements
//     #define MAX_NAMED_VARS         65536
//     #define MAX_DATA_ITEMS       1048576
//     // Estimated total: ~4 GB
//
//   Which pools to grow FIRST for maximum benefit:
//     1. MAX_STRING_POOL -- string-heavy programs benefit most.
//     2. MAX_ARRAY_ELEMENTS -- for large data sets / matrices.
//     3. VARIABLE_MEMORY_SIZE -- for many named variables.
//     4. MAX_PROGRAM_LINES -- for very long programs.
//     5. SCRATCH_MEMORY_SIZE -- rarely a bottleneck unless you
//        have expressions with 50+ nested function calls.
//     6. PROGRAM_MEMORY_SIZE -- stores the actual source text;
//        only matters for extremely long source lines.
//
//   Rule of thumb:
//     Keep BASIC++ total footprint under 5% of system RAM for
//     a comfortable experience. On a 16 GB machine, 350 MB
//     leaves >15 GB for the OS, editor, and other tools.
//
//   How to apply these changes:
//     1. Open config.h in the "MODERN BUILD" section (line ~259).
//     2. Replace the #define constants with your chosen values.
//     3. Recompile: build.bat (Windows) or make (Linux).
//     4. Run with -v flag to see pool sizes in boot output:
//          basicpp -v
//
//   WARNING: All pool sizes must be positive and fit in 'long'.
//   On 32-bit systems, individual pool sizes are capped at 2 GB
//   (2147483647 bytes). On 64-bit systems, there is no practical
//   limit beyond available virtual memory.
//
// ===================================================================
//
// MINIMALIZATION:
//   For a minimal build (FreeDOS, ESP32, etc.):
//   - All pool management functions are required (CORE tier).
//   - Reduce config.h pool sizes to fit your memory budget.
//   - The program store uses a flat array; no heap fragmentation.
//   - The smallest viable configuration uses ~38 KB total
//     (see config.h BPP_EMBEDDED profile).
//   - On ESP32 with PSRAM: define BPP_PSRAM for larger pools
//     (see config.h for PSRAM-aware defaults).
//
// TROUBLESHOOTING:
//   - "SORRY" at startup:
//     malloc() failed. Reduce pool sizes in config.h or free
//     system memory. The BPP_FREEDOS or BPP_EMBEDDED profiles
//     have much smaller defaults.
//
//   - "SORRY" during program editing:
//     MAX_PROGRAM_LINES reached. Increase it in config.h or
//     split the program using CHAIN/MERGE.
//
//   - "SORRY" during RUN:
//     Variable pool exhausted. Increase VARIABLE_MEMORY_SIZE
//     in config.h. Use VARS to check variable count.
//
//   - "Out of string space":
//     MAX_STRING_POOL exhausted. Increase it in config.h.
//     Use FRE("") in BASIC to check remaining string space.
//
//   - Adding custom error codes:
//     Error codes are defined in errors.h (ERR_WHAT, ERR_HOW,
//     ERR_SORRY). To add a new error code:
//     1. Add ERR_YOURERROR to the enum in errors.h.
//     2. Add the message string in errors.c error_messages[].
//     3. Raise it with error_raise(ERR_YOURERROR, line_num).
//
// PERFORMANCE:
//   - Bump allocation is O(1) -- a pointer increment.
//   - Pool reset is O(1) -- a single assignment.
//   - Binary search for GOTO is O(log n).
//   - Line insertion/deletion is O(n) due to memmove.
//   - Sequential iteration (RUN) is cache-friendly because
//     program lines are stored contiguously in memory.
//   - Pool zeroing at init is O(n) but happens only once.
//   - Larger pools do NOT slow down execution -- the watermark
//     only advances as far as needed. Unused pool space is
//     never touched and doesn't affect cache.
//
// DEPENDENCIES:
//   - config.h  (pool sizes, limits)
//   - memory.h  (MemoryPool, MemorySystem, ProgramStore structs)
//   - errors.h  (error_raise for capacity failures)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library functions (malloc, free,
//     memset, strncpy, printf).
//   - No platform-specific code paths.
//   - Compiles cleanly with gcc -std=c17 -Wall -Wextra and
//     MSVC /std:c17 /W4.
//
// ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "errors.h"

// -----------------------------------------------------------------
// Pool Management -- Internal Helpers
// -----------------------------------------------------------------
// These static functions handle the lifecycle of a single pool.
// They are called by mem_init() and mem_shutdown() only.
// -----------------------------------------------------------------

// init_pool - Allocate and initialize one memory pool.
//
// Allocates 'size' bytes via malloc() and zeroes the watermark.
// The memory is zeroed so that BASIC variables and array elements
// start at zero (matching classic BASIC convention where all
// variables are implicitly initialized to 0).
//
// Parameters:
//   pool - pointer to the MemoryPool to initialize
//   size - number of bytes to allocate
//
// Returns:
//   0 on success
//  -1 on allocation failure (pool fields set to safe defaults)
//
// Error behavior:
//   Does NOT call error_raise(). The caller (mem_init) handles
//   cascading cleanup and error reporting.
//
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

    // Zero the pool memory. This ensures variables and array
    // elements start at zero, matching BASIC convention.
    memset(pool->base, 0, (size_t)size);
    return 0;
}

// free_pool - Release one memory pool.
//
// Safe to call on an uninitialized or already-freed pool
// (checks for NULL before calling free). After this call,
// all pointers into the pool are invalid.
//
static void free_pool(MemoryPool *pool)
{
    if (pool->base != NULL) {
        free(pool->base);
        pool->base = NULL;
    }
    pool->size = 0;
    pool->used = 0;
}

// -----------------------------------------------------------------
// Pool Management -- Public API
// -----------------------------------------------------------------

// mem_init - Allocate all memory pools and the program store.
//
// This is the first function called during interpreter startup
// (from boot.c). It allocates three resources:
//
//   1. Variable pool  -- stores A-Z variables and the @() array.
//      Size: VARIABLE_MEMORY_SIZE from config.h.
//
//   2. Scratch pool   -- temporary workspace for tokenization
//      and expression parsing. Reset between statements.
//      Size: SCRATCH_MEMORY_SIZE from config.h.
//
//   3. Program line array -- sorted array of ProgramLine structs
//      for storing the BASIC program text. Each entry holds a
//      line number and up to MAX_LINE_LENGTH characters of text.
//      Capacity: MAX_PROGRAM_LINES from config.h.
//
// Allocation order matters: if any allocation fails, all
// previously allocated resources are freed before returning -1.
// This prevents memory leaks on partial initialization.
//
// Parameters:
//   mem - pointer to the MemorySystem to initialize
//
// Returns:
//   0 on success (all pools allocated)
//  -1 on failure (all pools freed, caller should report ERR_SORRY)
//
// How to customize:
//   To add a new pool, add its init_pool() call here and its
//   free_pool() call in mem_shutdown(). Update the failure
//   cleanup to free in reverse order.
//
int mem_init(MemorySystem *mem)
{
    // Initialize all fields to safe defaults before any malloc.
    // This ensures mem_shutdown() is safe even if we fail early.
    mem->variable.base = NULL;
    mem->variable.size = 0;
    mem->variable.used = 0;
    mem->scratch.base = NULL;
    mem->scratch.size = 0;
    mem->scratch.used = 0;
    mem->program.lines = NULL;
    mem->program.count = 0;
    mem->program.capacity = 0;

    // Allocate variable pool
    if (init_pool(&mem->variable, VARIABLE_MEMORY_SIZE) != 0) {
        return -1;
    }

    // Allocate scratch pool
    if (init_pool(&mem->scratch, SCRATCH_MEMORY_SIZE) != 0) {
        free_pool(&mem->variable);
        return -1;
    }

    // Allocate program line array.
    //
    // We use a separate malloc for the line array rather than
    // carving it from a pool because ProgramLine structs are
    // relatively large (~260 bytes each) and the array needs
    // to be contiguous for memmove operations during insert/delete.
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

    // Zero the line array for clean initial state.
    // All line_number fields start at 0 and all text buffers
    // start as empty strings.
    memset(mem->program.lines, 0,
        (size_t)MAX_PROGRAM_LINES * sizeof(ProgramLine));

    return 0;
}

// mem_shutdown - Free all pools and the program store.
//
// Called during interpreter shutdown (from boot_shutdown in boot.c).
// After this call, the MemorySystem is in a safe but unusable state.
// All pointers previously obtained from mem_pool_alloc() are invalid.
//
// Safe to call multiple times (idempotent).
//
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

// mem_pool_alloc - Bump-allocate bytes from a pool.
//
// Advances the watermark by 'nbytes' and returns a pointer to
// the start of the allocated region. The returned memory was
// zeroed when the pool was initialized (init_pool), but may
// contain stale data if the pool has been reset and reused.
//
// Parameters:
//   pool   - the pool to allocate from
//   nbytes - number of bytes to allocate (must be > 0)
//
// Returns:
//   Pointer to the allocated region within the pool.
//   NULL if nbytes <= 0 or if there is insufficient space.
//
// Error behavior:
//   Does NOT call error_raise(). The caller is responsible for
//   checking the return value and raising ERR_SORRY if needed.
//   This allows callers to attempt allocation without triggering
//   an error (e.g., for optional features that degrade gracefully).
//
// Alignment:
//   No alignment guarantee beyond char alignment. For long-aligned
//   access, the caller should request a size that is a multiple of
//   sizeof(long). In practice, the variable and scratch pools are
//   used for BValue structs which have natural alignment.
//
// Performance:
//   O(1) -- a bounds check and a pointer increment.
//
void *mem_pool_alloc(MemoryPool *pool, long nbytes)
{
    char *ptr;

    if (nbytes <= 0) {
        return NULL;
    }

    if (pool->used + nbytes > pool->size) {
        return NULL;  // insufficient space
    }

    ptr = pool->base + pool->used;
    pool->used += nbytes;
    return (void *)ptr;
}

// mem_pool_reset - Reset a pool's watermark to zero.
//
// All previous allocations from this pool become invalid.
// The memory itself is NOT zeroed (for performance). This
// is acceptable because bump allocation always initializes
// the region it hands out, or the caller does.
//
// Primary use:
//   The scratch pool is reset between statements during
//   program execution. Each statement gets a clean scratch
//   space without the cost of zeroing memory.
//
// When NOT to call:
//   Never reset the variable pool during program execution.
//   Variables must persist across statements. The variable
//   pool is only reset by runtime_reset() (on RUN/NEW).
//
// Performance:
//   O(1) -- a single assignment.
//
void mem_pool_reset(MemoryPool *pool)
{
    pool->used = 0;
}

// mem_pool_available - Return remaining bytes in a pool.
//
// Used by the SIZE function (runtime_size in runtime.c) and
// by capacity checks before large allocations.
//
// Returns:
//   Number of unallocated bytes remaining in the pool.
//   Always >= 0.
//
long mem_pool_available(MemoryPool *pool)
{
    return pool->size - pool->used;
}

// -----------------------------------------------------------------
// Program Store Operations
// -----------------------------------------------------------------
// The program store is a sorted array of ProgramLine entries.
// Each entry holds a line number (1-32767) and the full text
// of the line as entered by the user.
//
// Sorting invariant: lines[i].line_number < lines[i+1].line_number
// for all valid i. This invariant is maintained by program_insert()
// and program_delete().
//
// How to customize:
//   - To change the maximum line number range, edit
//     LINE_NUMBER_MIN and LINE_NUMBER_MAX in config.h.
//   - To change the maximum program size, edit
//     MAX_PROGRAM_LINES in config.h.
//   - To change the maximum line length, edit
//     MAX_LINE_LENGTH in config.h (default 255).
//
// Performance:
//   Binary search (find_insert_pos): O(log n)
//   Insert with shift: O(n)  [n = program line count]
//   Delete with shift: O(n)
//   Sequential scan (LIST, RUN): O(n), cache-friendly
// -----------------------------------------------------------------

// find_insert_pos - Binary search for insertion position.
//
// Returns the index where a line with the given number should be
// inserted to maintain sorted order. If a line with that number
// already exists, returns its index (the caller checks for
// replacement vs. insertion).
//
// This is a standard binary search on a sorted array of line
// numbers. The search space is lines[0..count-1].
//
// Parameters:
//   store       - the program store to search
//   line_number - the line number to find or locate
//
// Returns:
//   Index where the line number belongs (0 <= result <= count).
//   If an exact match exists, returns the index of the match.
//   If no match, returns the insertion point.
//
static int find_insert_pos(ProgramStore *store, int line_number)
{
    int low = 0;
    int high = store->count - 1;
    int mid;

    while (low <= high) {
        mid = low + (high - low) / 2;
        if (store->lines[mid].line_number == line_number) {
            return mid;  // exact match
        } else if (store->lines[mid].line_number < line_number) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return low;  // insertion point
}

// program_insert - Insert or replace a program line.
//
// If a line with the given number already exists, its text is
// replaced in-place (no reordering needed). If the line is new,
// a slot is opened at the correct sorted position by shifting
// subsequent lines up by one, and the new line is inserted.
//
// Parameters:
//   store       - the program store to modify
//   line_number - line number (1 to LINE_NUMBER_MAX)
//   full_text   - complete line text including the line number
//                 prefix (e.g., "10 PRINT \"HELLO\"")
//
// Returns:
//   0 on success
//  -1 if the store is full (raises ERR_SORRY)
//
// Error behavior:
//   Raises ERR_SORRY if the program store has reached
//   MAX_PROGRAM_LINES capacity. The line is not stored.
//
// How to customize:
//   To support longer lines, increase MAX_LINE_LENGTH in config.h.
//   To support more lines, increase MAX_PROGRAM_LINES in config.h
//   (costs ~260 bytes per additional slot).
//
int program_insert(ProgramStore *store, int line_number,
    const char *full_text)
{
    int pos;
    int i;

    pos = find_insert_pos(store, line_number);

    // Check if this is a replacement of an existing line
    if (pos < store->count &&
        store->lines[pos].line_number == line_number) {
        // Replace existing line text
        strncpy(store->lines[pos].text, full_text, MAX_LINE_LENGTH);
        store->lines[pos].text[MAX_LINE_LENGTH] = '\0';
        return 0;
    }

    // Inserting a new line -- check capacity
    if (store->count >= store->capacity) {
        error_raise(ERR_SORRY, 0);
        return -1;
    }

    // Shift lines from pos..count-1 up by one position.
    // We iterate backwards to avoid overwriting data.
    for (i = store->count; i > pos; i--) {
        store->lines[i] = store->lines[i - 1];
    }

    // Insert the new line at the correct sorted position
    store->lines[pos].line_number = line_number;
    strncpy(store->lines[pos].text, full_text, MAX_LINE_LENGTH);
    store->lines[pos].text[MAX_LINE_LENGTH] = '\0';
    store->count++;

    return 0;
}

// program_delete - Delete a line by number.
//
// Finds the line via binary search and removes it by shifting
// all subsequent lines down by one position to close the gap.
//
// Parameters:
//   store       - the program store to modify
//   line_number - line number to delete
//
// Returns:
//   0 on success
//  -1 if the line was not found (this is NOT an error in BASIC;
//      deleting a nonexistent line is silently ignored)
//
int program_delete(ProgramStore *store, int line_number)
{
    int pos;
    int i;

    pos = find_insert_pos(store, line_number);

    // Verify we found an exact match
    if (pos >= store->count ||
        store->lines[pos].line_number != line_number) {
        return -1;  // line not found (silent, not an error)
    }

    // Shift lines down to fill the gap
    for (i = pos; i < store->count - 1; i++) {
        store->lines[i] = store->lines[i + 1];
    }

    store->count--;
    return 0;
}

// program_find - Find a line by exact line number.
//
// Uses binary search to locate a line with the given number.
//
// Parameters:
//   store       - the program store to search
//   line_number - exact line number to find
//
// Returns:
//   Index into store->lines[] if found.
//  -1 if no line with that number exists.
//
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

// program_find_next - Find the first line with number >= target.
//
// Used by GOTO to locate the target line. If the exact line
// exists, returns its index. Otherwise returns the index of the
// next higher line number. Returns -1 if no line has a number
// >= target (i.e., the target is beyond all stored lines).
//
// This supports the PATB convention where GOTO to a nonexistent
// line number raises HOW? -- the caller uses this function to
// find the target and then checks for an exact match.
//
// Parameters:
//   store       - the program store to search
//   line_number - target line number
//
// Returns:
//   Index of the first line with number >= line_number.
//  -1 if no such line exists.
//
int program_find_next(ProgramStore *store, int line_number)
{
    int pos;

    pos = find_insert_pos(store, line_number);

    if (pos < store->count) {
        return pos;
    }

    return -1;  // no line at or after this number
}

// program_clear - Remove all stored lines (NEW command).
//
// Resets the line count to zero. The underlying array memory
// remains allocated and will be reused by subsequent program_insert()
// calls. This avoids the overhead of freeing and re-allocating
// the program store on every NEW command.
//
// The old line data is not zeroed (for performance). Binary search
// and iteration use 'count' as their bounds, so stale data beyond
// count is never accessed.
//
void program_clear(ProgramStore *store)
{
    store->count = 0;
}

// program_list - Print program lines to stdout (LIST command).
//
// Lists all stored lines with line numbers in the range [from, to].
// If from <= 0, starts from the first stored line.
// If to <= 0, lists through the last stored line.
//
// Output format:
//   Each line is printed exactly as stored (including the line
//   number prefix), followed by a newline. This preserves the
//   user's original formatting and keyword case mode.
//
// Parameters:
//   store - the program store to list
//   from  - first line number to include (0 = start of program)
//   to    - last line number to include (0 = end of program)
//
// How to customize:
//   To add line numbering, column markers, or syntax highlighting,
//   modify this function. The stored text is in store->lines[i].text.
//   The line number is in store->lines[i].line_number.
//
void program_list(ProgramStore *store, int from, int to)
{
    int i;

    for (i = 0; i < store->count; i++) {
        if (from > 0 && store->lines[i].line_number < from) {
            continue;
        }
        if (to > 0 && store->lines[i].line_number > to) {
            break;  // lines are sorted, so no more matches
        }
        printf("%s\n", store->lines[i].text);
    }
}
