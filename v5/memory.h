/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memory.h
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
 // BASIC++ Interpreter - memory.h
 // ---
 //
 // Memory management subsystem interface.
 //
 // PURPOSE:
 // Manages three explicit memory pools and provides a program
 // storage system. All dynamic memory used by the interpreter
 // flows through this module - no other module calls malloc()
 // directly (except for one-time setup allocations).
 //
 // MEMORY LAYOUT:
 // +---------------------------+
 // | Program Store (line array)| <- ProgramLine array, sorted by line#
 // +---------------------------+
 // | Variable Pool (64K) | <- A-Z variables + @() array
 // +---------------------------+
 // | Scratch Pool (64K) | <- temporary buffers, reset per stmt
 // +---------------------------+
 //
 // The program store is a separate allocation (array of ProgramLine
 // structs). The variable and scratch pools are flat byte arrays
 // with bump allocation.
 //
 // HOW TO EXTEND:
 // HOW TO EXTEND:
 //   Adding a new memory pool is a 4-step process:
 //   1. Add a new MemoryPool field to MemorySystem below.
 //   2. Add a size constant to config.h (per build profile).
 //   3. Add init_pool()/free_pool() calls in memory.c.
 //   4. Allocate from it using mem_pool_alloc() in your module.
 //
 //   All pool operations (alloc, reset, bounds check) work
 //   generically on any MemoryPool -- no special code needed.
 //
 //   For a complete step-by-step guide with worked examples
 //   (graphics framebuffer, sound buffer, call stack, network
 //   I/O buffer, user types), see memory.c section:
 //   "ADDING A NEW MEMORY POOL -- Step-by-Step Guide"
 //
 // ---

#ifndef BASICPP_MEMORY_H
#define BASICPP_MEMORY_H

#include "config.h"

// --- MemoryPool - A single flat memory arena. ---
 // Each pool is a contiguous block of bytes allocated via malloc().
 // Allocation within the pool uses bump allocation: a 'used' watermark
 // advances with each allocation. The watermark can be reset to zero
 // to reclaim all space (used for the scratch pool between statements).
 //
 // Fields:
 // base - pointer to the start of the allocated block.
 // size - total size of the block in bytes.
 // used - current watermark (bytes allocated so far).
typedef struct MemoryPool {
 char *base;
 long size;
 long used;
} MemoryPool;

// --- ProgramLine - A single stored BASIC program line. ---
 // Stores the line number and the exact source text as entered by
 // the user. No tokenization or transformation is applied to stored
 // lines - LIST and SAVE reproduce them verbatim.
 //
 // Fields:
 // line_number - the BASIC line number (1-32767).
 // text - the full line text including the line number prefix.
 // Null-terminated, max MAX_LINE_LENGTH characters.
typedef struct ProgramLine {
 double line_number;
 char *text;
} ProgramLine;

// --- ProgramStore - Indexed collection of program lines. ---
 // Lines are stored in a dynamically allocated array, kept sorted
 // by line_number at all times. This allows efficient binary search
 // for GOTO targets and sequential iteration for RUN.
 //
 // Fields:
 // lines - pointer to the array of ProgramLine structs.
 // count - number of lines currently stored.
 // capacity - maximum number of lines (MAX_PROGRAM_LINES).
typedef struct ProgramStore {
 ProgramLine *lines;
 int count;
 int capacity;
 char *bulk_buffer;
 size_t bulk_size;
} ProgramStore;

#define RAMBANK_SIZE 1048576  // 1MB
#define MAX_RESIDENT_BANKS 8  // Resident limit for physical RAM paging simulation
extern int g_init_rambanks;

typedef struct RamBank {
    char *base;          // Pointer to 1MB bank buffer if resident (otherwise NULL)
    int id;              // Bank ID (1-254)
    int resident;        // Is the bank loaded in resident memory?
    int dirty;           // Has it been modified since loaded?
    int shared;          // Is the bank shared/accessible across tasks?
    long last_access;    // Counter for LRU paging eviction
} RamBank;

// --- MemorySystem - Top-level container for all memory pools. ---
 // Owns the variable pool and scratch pool. The program store is
 // a separate structure because it uses structured storage (array
 // of ProgramLine) rather than flat byte allocation.
typedef struct MemorySystem {
 MemoryPool variable;
 MemoryPool scratch;
 MemoryPool graphics;
 ProgramStore program;
 // Segmented Virtual Memory RAMBANKs
 int num_rambanks;
 RamBank *banks;
 long access_counter;    // Incrementing counter for LRU
} MemorySystem;

// --- Pool Management Functions ---

 // mem_init - Allocate and initialize all memory pools.
 //
 // Allocates the variable pool, scratch pool, and program line array
 // via malloc(). Returns 0 on success, -1 on failure (any allocation
 // failure causes all pools to be freed and returns error).
 //
 // Must be called once at startup before any other memory operations.
int mem_init(MemorySystem *mem);

 // mem_shutdown - Free all memory pools.
 //
 // Releases all dynamically allocated memory. Safe to call even if
 // mem_init() failed partway through (handles NULL pointers).
 // Must be called at shutdown.
void mem_shutdown(MemorySystem *mem);

 // mem_pool_alloc - Allocate bytes from a specific pool.
 //
 // Bump-allocates 'nbytes' from the given pool. Returns a pointer
 // to the allocated region, or NULL if insufficient space remains.
 //
 // The returned pointer is valid until the pool is reset or freed.
 // Alignment is not guaranteed beyond char alignment - callers
 // needing long alignment should round up nbytes appropriately.
void *mem_pool_alloc(MemoryPool *pool, long nbytes);

 // mem_pool_reset - Reset a pool's watermark to zero.
 //
 // Effectively "frees" all allocations from the pool. Used for the
 // scratch pool between statements. Does not zero the memory.
void mem_pool_reset(MemoryPool *pool);

 // mem_pool_available - Return bytes remaining in a pool.
 //
 // Used to implement the SIZE function (reports free variable pool
 // space) and for capacity checks.
long mem_pool_available(MemoryPool *pool);

// --- Program Store Functions ---

double program_parse_line_number(const char *input, int *end_pos, int *is_non_decimal);

 // program_insert - Insert or replace a program line.
 //
 // If a line with the given number already exists, it is replaced.
 // Otherwise, a new line is inserted in sorted order. The full_text
 // parameter is the complete line as entered (including line number).
 //
 // Returns 0 on success, -1 if the program store is full (ERR_SORRY).
int program_insert(ProgramStore *store, double line_number,
 const char *full_text);

 // program_insert_pointer - Insert or replace a program line using a pre-allocated pointer.
int program_insert_pointer(ProgramStore *store, double line_number,
 char *text_ptr);

 // program_delete - Delete a program line by number.
 //
 // Removes the line with the given number, shifting subsequent lines
 // down. Returns 0 on success, -1 if the line was not found.
int program_delete(ProgramStore *store, double line_number);

 // program_find - Find a line by exact line number.
 //
 // Returns the index into the lines array, or -1 if not found.
 // Uses binary search for efficiency.
int program_find(ProgramStore *store, double line_number);

 // program_find_next - Find the first line with number >= target.
 //
 // Used by GOTO to find the target line. Returns the index, or -1
 // if no line has a number >= target. Uses binary search.
int program_find_next(ProgramStore *store, double line_number);

 // program_clear - Remove all stored lines (NEW command).
 //
 // Resets the line count to zero. Does not free the array itself.
void program_clear(ProgramStore *store);

 // program_list - Print stored lines to stdout (LIST command).
 //
 // Prints all lines with line numbers between 'from' and 'to'
 // (inclusive). If from <= 0, starts from the first line.
 // If to <= 0, continues to the last line.
void program_list(ProgramStore *store, double from, double to);

#define SEARCH_SUBSTRING      1
#define SEARCH_IDENTIFIER     2
#define SEARCH_LABEL_OR_FUNC  3


#define SEARCH_SUBSTRING_CS   4

typedef struct {
    double from;
    double to;
} ListRange;

void program_list_search(ProgramStore *store, int search_type, const char *pattern);
void program_list_complex(ProgramStore *store, ListRange *ranges, int range_count, int search_type, const char *pattern);


// --- RAMBANK Segmented Virtual Memory Functions ---
void rambank_init(MemorySystem *mem);
void rambank_shutdown(MemorySystem *mem);
void rambank_ensure_resident(MemorySystem *mem, int bank_id);
unsigned char rambank_peek(MemorySystem *mem, int bank_id, long offset, int line_num);
void rambank_poke(MemorySystem *mem, int bank_id, long offset, unsigned char value, int line_num);
long rambank_free_space(MemorySystem *mem, int bank_id);
void rambank_set_shared(MemorySystem *mem, int bank_id, int shared);
void rambank_copy(MemorySystem *mem, int src_bank, long src_offset, int dst_bank, long dst_offset, long length, int line_num);
void rambank_fill(MemorySystem *mem, int bank_id, long offset, long length, unsigned char value, int line_num);

#endif // BASICPP_MEMORY_H
