/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mem_system.c
 * @brief Engine Memory Context, Arena Allocator, and Program Line Buffer implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `mem_create()`, `mem_destroy()`, `mem_scratch_alloc()`, `mem_scratch_reset()`, `mem_store_line()`, `mem_get_line()`, `mem_delete_line()`, managing statement scratch arenas and line tables.
 *
 * 2. WHY IT EXISTS:
 * Encapsulates all heap memory management within bounded memory pools (e.g. 640 MB for `baspp`, 384 MB for `bpp`, 64 MB for `bs`), guaranteeing safe zero-initialization and clean resets.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Uses a bump allocator for statement scratch memory (reset per line) and a sorted line array for program storage using binary search for O(log N) line lookups.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "memory/memory.h", "types/config.h", <stdlib.h>, <string.h>, <stdio.h>, <stdint.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Adjust default pool capacities or add specialized subsystem pools.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory 8-byte alignment for doubles and 64-bit pointers; zero-initialization on allocation (`calloc` / `memset`).
 *
 * 8. WHAT TO EXPECT:
 * `mem_scratch_alloc()` returns 8-byte aligned memory from the statement scratch pool; `mem_scratch_reset()` invalidates all scratch pointers in O(1).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Trace binary search bounds in `mem_get_line()` or inspect pool exhaustion in `mem_scratch_alloc()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `MemoryContext` pointer initialized via `mem_create()`.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses `uintptr_t` for alignment arithmetic.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/types/config.c
 * Prerequisite Header Files:
 * - engine/include/memory/memory.h
 * - engine/include/types/config.h
 */

#include "memory/memory.h"
#include "types/config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Private definition of the MemoryContext */
struct MemoryContext {
    /* Scratch Bump Arena */
    char   *scratch_base;
    size_t  scratch_size;
    size_t  scratch_used;

    /* String Heap Tracking */
    size_t  str_limit;
    size_t  str_used;

    /* Variable Pool */
    char   *var_base;
    size_t  var_size;
    size_t  var_used;

    /* Program Line Store */
    BppProgramLine *lines;
    size_t          lines_count;
    size_t          lines_capacity;
    size_t          lines_mem_used; /* Tracks text buffer memory usage */
    size_t          lines_mem_limit;

    /* Library Program Line Store */
    BppProgramLine *lib_lines;
    size_t          lib_lines_count;
    size_t          lib_lines_capacity;

    /* Program Version Tag */
    char            program_version[32];
};

/* Align size to 8-byte boundary for performance and safety */
static inline size_t align8(size_t size) {
    return (size + 7) & ~7;
}

MemoryContext *mem_init(size_t prog_mem_sz, size_t var_mem_sz, size_t str_mem_sz, size_t scratch_mem_sz) {
    MemoryContext *ctx = (MemoryContext *)calloc(1, sizeof(MemoryContext));
    if (!ctx) return NULL;

    /* Allocate scratch arena */
    ctx->scratch_size = scratch_mem_sz;
    ctx->scratch_base = (char *)calloc(1, ctx->scratch_size);
    if (!ctx->scratch_base) {
        free(ctx);
        return NULL;
    }
    ctx->scratch_used = 0;

    /* Set up string limit */
    ctx->str_limit = str_mem_sz;
    ctx->str_used = 0;

    /* Allocate variable space */
    ctx->var_size = var_mem_sz;
    ctx->var_base = (char *)calloc(1, ctx->var_size);
    if (!ctx->var_base) {
        free(ctx->scratch_base);
        free(ctx);
        return NULL;
    }
    ctx->var_used = 0;

    /* Initialize program store */
    ctx->lines_mem_limit = prog_mem_sz;
    ctx->lines_capacity = 128;
    ctx->lines = (BppProgramLine *)calloc(ctx->lines_capacity, sizeof(BppProgramLine));
    if (!ctx->lines) {
        free(ctx->var_base);
        free(ctx->scratch_base);
        free(ctx);
        return NULL;
    }
    ctx->lines_count = 0;
    ctx->lines_mem_used = 0;

    /* Initialize library program store */
    ctx->lib_lines_capacity = 128;
    ctx->lib_lines = (BppProgramLine *)calloc(ctx->lib_lines_capacity, sizeof(BppProgramLine));
    if (!ctx->lib_lines) {
        free(ctx->lines);
        free(ctx->var_base);
        free(ctx->scratch_base);
        free(ctx);
        return NULL;
    }
    ctx->lib_lines_count = 0;

    return ctx;
}

void mem_shutdown(MemoryContext *ctx) {
    if (!ctx) return;

    /* Free all program lines */
    mem_program_clear(ctx);
    free(ctx->lines);

    /* Free all library lines */
    mem_lib_program_clear(ctx);
    free(ctx->lib_lines);

    /* Free arenas */
    free(ctx->var_base);
    free(ctx->scratch_base);
    free(ctx);
}

void *mem_scratch_alloc(MemoryContext *ctx, size_t size) {
    if (!ctx) return NULL;
    size = align8(size);
    if (size > ctx->scratch_size || ctx->scratch_used > ctx->scratch_size - size) {
        /* Scratch OOM */
        return NULL;
    }
    void *ptr = ctx->scratch_base + ctx->scratch_used;
    ctx->scratch_used += size;
    return ptr;
}

void mem_scratch_reset(MemoryContext *ctx) {
    if (ctx) {
        ctx->scratch_used = 0;
    }
}

/* Binary search helper to find the index of a line or where it should be inserted */
static bool find_line_index(MemoryContext *ctx, BppLineNumber line, size_t *out_idx) {
    if (ctx->lines_count == 0) {
        *out_idx = 0;
        return false;
    }
    size_t low = 0;
    size_t high = ctx->lines_count - 1;

    while (low <= high) {
        size_t mid = low + (high - low) / 2;
        if (ctx->lines[mid].line_number == line) {
            *out_idx = mid;
            return true;
        }
        if (ctx->lines[mid].line_number < line) {
            low = mid + 1;
        } else {
            if (mid == 0) break;
            high = mid - 1;
        }
    }
    *out_idx = low;
    return false;
}

bool mem_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text) {
    if (!ctx || !text) return false;

    size_t len = strlen(text);
    size_t idx = 0;
    bool exists = find_line_index(ctx, line, &idx);

    if (exists) {
        /* Replace existing line */
        size_t old_len = strlen(ctx->lines[idx].text);
        if (ctx->lines_mem_used - old_len + len > ctx->lines_mem_limit) {
            return false; /* OOM program limit exceeded */
        }
        char *new_text = (char *)realloc(ctx->lines[idx].text, len + 1);
        if (!new_text) return false;
        strcpy(new_text, text);
        ctx->lines[idx].text = new_text;
        ctx->lines_mem_used = ctx->lines_mem_used - old_len + len;
        return true;
    }

    /* Insert new line */
    if (ctx->lines_mem_used + len + sizeof(BppProgramLine) > ctx->lines_mem_limit) {
        return false; /* OOM limit exceeded */
    }

    if (ctx->lines_count >= ctx->lines_capacity) {
        size_t new_cap = ctx->lines_capacity * 2;
        BppProgramLine *new_lines = (BppProgramLine *)realloc(ctx->lines, new_cap * sizeof(BppProgramLine));
        if (!new_lines) return false;
        ctx->lines = new_lines;
        ctx->lines_capacity = new_cap;
    }

    /* Allocate line text buffer */
    char *text_copy = (char *)calloc(1, len + 1);
    if (!text_copy) return false;
    strcpy(text_copy, text);

    /* Shift lines to make room for insertion */
    for (size_t i = ctx->lines_count; i > idx; --i) {
        ctx->lines[i] = ctx->lines[i - 1];
    }

    ctx->lines[idx].line_number = line;
    ctx->lines[idx].text = text_copy;
    ctx->lines_count++;
    ctx->lines_mem_used += (len + sizeof(BppProgramLine));

    return true;
}

bool mem_program_delete(MemoryContext *ctx, BppLineNumber line) {
    if (!ctx) return false;
    size_t idx = 0;
    if (!find_line_index(ctx, line, &idx)) {
        return false; /* Not found */
    }

    /* Free memory for this line */
    size_t len = strlen(ctx->lines[idx].text);
    free(ctx->lines[idx].text);
    ctx->lines_mem_used -= (len + sizeof(BppProgramLine));

    /* Shift remaining lines */
    for (size_t i = idx; i < ctx->lines_count - 1; ++i) {
        ctx->lines[i] = ctx->lines[i + 1];
    }
    ctx->lines_count--;

    return true;
}

const char *mem_program_get(MemoryContext *ctx, BppLineNumber line) {
    if (!ctx) return NULL;
    size_t idx = 0;
    if (find_line_index(ctx, line, &idx)) {
        return ctx->lines[idx].text;
    }
    return NULL;
}

BppProgramLine *mem_program_get_all(MemoryContext *ctx, size_t *count) {
    if (!ctx || !count) return NULL;
    *count = ctx->lines_count;
    return ctx->lines;
}

void mem_program_clear(MemoryContext *ctx) {
    if (!ctx) return;
    for (size_t i = 0; i < ctx->lines_count; ++i) {
        free(ctx->lines[i].text);
    }
    ctx->lines_count = 0;
    ctx->lines_mem_used = 0;
    ctx->program_version[0] = '\0';
}

void mem_program_set_version(MemoryContext *ctx, const char *ver_str) {
    if (!ctx) return;
    if (!ver_str) {
        ctx->program_version[0] = '\0';
        return;
    }
    strncpy(ctx->program_version, ver_str, sizeof(ctx->program_version) - 1);
    ctx->program_version[sizeof(ctx->program_version) - 1] = '\0';
}

const char *mem_program_get_version(MemoryContext *ctx) {
    if (!ctx) return "";
    return ctx->program_version;
}

void *mem_string_alloc(MemoryContext *ctx, size_t size) {
    if (!ctx) return NULL;
    if (size > SIZE_MAX - sizeof(size_t)) return NULL;
    size_t total_size = size + sizeof(size_t);
    if (total_size > ctx->str_limit || ctx->str_used > ctx->str_limit - total_size) {
        return NULL; /* String heap limit */
    }
    size_t *ptr = (size_t *)calloc(1, total_size);
    if (!ptr) return NULL;
    *ptr = total_size;
    ctx->str_used += total_size;
    return (void *)(ptr + 1);
}

void mem_string_free(MemoryContext *ctx, void *ptr) {
    if (!ctx || !ptr) return;
    /* In a full implementation, we need to know the block size.
     * For now, string allocations store block sizes prefixing data,
     * or string system tracks it. We'll deduct it properly.
     * Let's assume size is managed by caller or pass it in.
     * For tracking, we can just malloc and trace.
     * Let's let the caller track exact sizes, or we wrap it with a prefix.
     * Prefixing size is extremely safe:
     */
    size_t *prefix = (size_t *)((char *)ptr - sizeof(size_t));
    size_t size = *prefix;
    ctx->str_used -= size;
    free(prefix);
}

size_t mem_get_free_ram(MemoryContext *ctx) {
    if (!ctx) return 0;
    /* Dynamic memory left across variables, strings, scratch, and programs */
    size_t limit = ctx->lines_mem_limit + ctx->var_size + ctx->str_limit + ctx->scratch_size;
    size_t used = ctx->lines_mem_used + ctx->var_used + ctx->str_used + ctx->scratch_used;
    return (limit > used) ? (limit - used) : 0;
}

size_t mem_get_used_ram(MemoryContext *ctx) {
    if (!ctx) return 0;
    return ctx->lines_mem_used + ctx->var_used + ctx->str_used + ctx->scratch_used;
}

void mem_format_size(size_t bytes, char *buf, size_t buf_size) {
    if (!buf || buf_size == 0) return;
    double size = (double)bytes;
    const char *units[] = {"Bytes", "KB", "MB", "GB"};
    int idx = 0;
    while (size >= 1024.0 && idx < 3) {
        size /= 1024.0;
        idx++;
    }
    if (idx == 0) {
        snprintf(buf, buf_size, "%lu Bytes", (unsigned long)bytes);
    } else {
        snprintf(buf, buf_size, "%.2f %s", size, units[idx]);
    }
}

bool mem_lib_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text) {
    if (!ctx || !text) return false;
    size_t len = strlen(text);

    if (ctx->lib_lines_count >= ctx->lib_lines_capacity) {
        size_t new_cap = ctx->lib_lines_capacity * 2;
        BppProgramLine *new_lines = (BppProgramLine *)realloc(ctx->lib_lines, new_cap * sizeof(BppProgramLine));
        if (!new_lines) return false;
        ctx->lib_lines = new_lines;
        ctx->lib_lines_capacity = new_cap;
    }

    char *text_copy = (char *)calloc(1, len + 1);
    if (!text_copy) return false;
    strcpy(text_copy, text);

    ctx->lib_lines[ctx->lib_lines_count].line_number = line;
    ctx->lib_lines[ctx->lib_lines_count].text = text_copy;
    ctx->lib_lines_count++;
    return true;
}

BppProgramLine *mem_lib_program_get_all(MemoryContext *ctx, size_t *count) {
    if (!ctx || !count) return NULL;
    *count = ctx->lib_lines_count;
    return ctx->lib_lines;
}

void mem_lib_program_clear(MemoryContext *ctx) {
    if (!ctx) return;
    for (size_t i = 0; i < ctx->lib_lines_count; ++i) {
        free(ctx->lib_lines[i].text);
    }
    ctx->lib_lines_count = 0;
}

