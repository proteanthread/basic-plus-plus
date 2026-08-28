// FILENAME: mem_system.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (memory.h, memory.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libkernel (config.h)
// Provides core logic and interface definitions for mem_system within BASIC++.
//
// ---- Includes ----

#include "memory/memory.h"
#include "types/config.h"
#include <stdint.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

extern void eval_ast_free_tree(void *node);

// Private definition of the MemoryContext
struct MemoryContext {
    // Scratch Bump Arena
    char   *scratch_base;
    size_t  scratch_size;
    size_t  scratch_used;

    // String Heap Tracking
    size_t  str_limit;
    size_t  str_used;

    // Variable Pool
    char   *var_base;
    size_t  var_size;
    size_t  var_used;

    // Program Line Store
    BppProgramLine *lines;
    size_t          lines_count;
    size_t          lines_capacity;
    size_t          lines_mem_used; // Tracks text buffer memory usage
    size_t          lines_mem_limit;

    // Library Program Line Store
    BppProgramLine *lib_lines;
    size_t          lib_lines_count;
    size_t          lib_lines_capacity;

    // Program Version Tag
    char            program_version[32];

    // Namespace tracking flag for fast bypass
    bool            has_namespaces;
};

// Align size to 8-byte boundary for performance and safety
static inline size_t align8(size_t size) {
    return (size + 7) & ~7;
}

MemoryContext *mem_init(size_t prog_mem_sz, size_t var_mem_sz, size_t str_mem_sz, size_t scratch_mem_sz) {
    HalContext *hal = hal_get();
    MemoryContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (MemoryContext *)hal->mem.alloc(sizeof(MemoryContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(MemoryContext));

    // Allocate scratch arena
    ctx->scratch_size = scratch_mem_sz;
    if (hal && hal->mem.alloc) {
        ctx->scratch_base = (char *)hal->mem.alloc(ctx->scratch_size);
    }
    if (!ctx->scratch_base) {
        if (hal && hal->mem.free) hal->mem.free(ctx);
        return NULL;
    }
    runtime_memset(ctx->scratch_base, 0, ctx->scratch_size);
    ctx->scratch_used = 0;

    // Set up string limit
    ctx->str_limit = str_mem_sz;
    ctx->str_used = 0;

    // Allocate variable space
    ctx->var_size = var_mem_sz;
    if (hal && hal->mem.alloc) {
        ctx->var_base = (char *)hal->mem.alloc(ctx->var_size);
    }
    if (!ctx->var_base) {
        if (hal && hal->mem.free) {
            hal->mem.free(ctx->scratch_base);
            hal->mem.free(ctx);
        }
        return NULL;
    }
    runtime_memset(ctx->var_base, 0, ctx->var_size);
    ctx->var_used = 0;

    // Initialize program store
    ctx->lines_mem_limit = prog_mem_sz;
    ctx->lines_capacity = 128;
    if (hal && hal->mem.alloc) {
        ctx->lines = (BppProgramLine *)hal->mem.alloc(ctx->lines_capacity * sizeof(BppProgramLine));
    }
    if (!ctx->lines) {
        if (hal && hal->mem.free) {
            hal->mem.free(ctx->var_base);
            hal->mem.free(ctx->scratch_base);
            hal->mem.free(ctx);
        }
        return NULL;
    }
    runtime_memset(ctx->lines, 0, ctx->lines_capacity * sizeof(BppProgramLine));
    ctx->lines_count = 0;
    ctx->lines_mem_used = 0;

    // Initialize library program store
    ctx->lib_lines_capacity = 128;
    if (hal && hal->mem.alloc) {
        ctx->lib_lines = (BppProgramLine *)hal->mem.alloc(ctx->lib_lines_capacity * sizeof(BppProgramLine));
    }
    if (!ctx->lib_lines) {
        if (hal && hal->mem.free) {
            hal->mem.free(ctx->lines);
            hal->mem.free(ctx->var_base);
            hal->mem.free(ctx->scratch_base);
            hal->mem.free(ctx);
        }
        return NULL;
    }
    runtime_memset(ctx->lib_lines, 0, ctx->lib_lines_capacity * sizeof(BppProgramLine));
    ctx->lib_lines_count = 0;

    return ctx;
}

void mem_shutdown(MemoryContext *ctx) {
    if (!ctx) return;
    HalContext *hal = hal_get();

    // Free all program lines
    mem_program_clear(ctx);
    if (ctx->lines && hal && hal->mem.free) {
        hal->mem.free(ctx->lines);
        ctx->lines = NULL;
    }

    // Free all library lines
    mem_lib_program_clear(ctx);
    if (ctx->lib_lines && hal && hal->mem.free) {
        hal->mem.free(ctx->lib_lines);
        ctx->lib_lines = NULL;
    }

    // Free arenas
    if (hal && hal->mem.free) {
        if (ctx->var_base) hal->mem.free(ctx->var_base);
        if (ctx->scratch_base) hal->mem.free(ctx->scratch_base);
        hal->mem.free(ctx);
    }
}


void *mem_scratch_alloc(MemoryContext *ctx, size_t size) {
    if (!ctx) return NULL;
    size = align8(size);
    if (size > ctx->scratch_size || ctx->scratch_used > ctx->scratch_size - size) {
        // Scratch OOM
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

// Binary search helper to find the index of a line or where it should be inserted
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
    HalContext *hal = hal_get();

    size_t len = runtime_strlen(text);
    size_t idx = 0;
    bool exists = find_line_index(ctx, line, &idx);

    if (exists) {
        // Replace existing line
        size_t old_len = runtime_strlen(ctx->lines[idx].text);
        if (ctx->lines_mem_used - old_len + len > ctx->lines_mem_limit) {
            return false; // OOM program limit exceeded
        }
        char *new_text = NULL;
        if (hal && hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(ctx->lines[idx].text, len + 1);
        } else if (hal && hal->mem.alloc) {
            new_text = (char *)hal->mem.alloc(len + 1);
            if (new_text && ctx->lines[idx].text) {
                runtime_memcpy(new_text, ctx->lines[idx].text, old_len + 1);
                if (hal->mem.free) hal->mem.free(ctx->lines[idx].text);
            }
        }
        if (!new_text) return false;
        runtime_strcpy(new_text, text);
        ctx->lines[idx].text = new_text;
        ctx->lines_mem_used = ctx->lines_mem_used - old_len + len;
        if (ctx->lines[idx].ast_cache) {
            eval_ast_free_tree(ctx->lines[idx].ast_cache);
            ctx->lines[idx].ast_cache = NULL;
            ctx->lines[idx].ast_valid = false;
        }
        return true;
    }

    // Insert new line
    if (ctx->lines_mem_used + len + sizeof(BppProgramLine) > ctx->lines_mem_limit) {
        return false; // OOM limit exceeded
    }

    if (ctx->lines_count >= ctx->lines_capacity) {
        size_t old_cap = ctx->lines_capacity;
        size_t new_cap = ctx->lines_capacity * 2;
        BppProgramLine *new_lines = NULL;
        if (hal && hal->mem.realloc) {
            new_lines = (BppProgramLine *)hal->mem.realloc(ctx->lines, new_cap * sizeof(BppProgramLine));
        } else if (hal && hal->mem.alloc) {
            new_lines = (BppProgramLine *)hal->mem.alloc(new_cap * sizeof(BppProgramLine));
            if (new_lines && ctx->lines) {
                runtime_memcpy(new_lines, ctx->lines, old_cap * sizeof(BppProgramLine));
                if (hal->mem.free) hal->mem.free(ctx->lines);
            }
        }
        if (!new_lines) return false;
        ctx->lines = new_lines;
        ctx->lines_capacity = new_cap;
    }

    // Allocate line text buffer
    char *text_copy = NULL;
    if (hal && hal->mem.alloc) {
        text_copy = (char *)hal->mem.alloc(len + 1);
    }
    if (!text_copy) return false;
    runtime_strcpy(text_copy, text);

    // Shift lines to make room for insertion
    for (size_t i = ctx->lines_count; i > idx; --i) {
        ctx->lines[i] = ctx->lines[i - 1];
    }

    ctx->lines[idx].line_number = line;
    ctx->lines[idx].text = text_copy;
    ctx->lines[idx].ast_cache = NULL;
    ctx->lines[idx].ast_valid = false;
    ctx->lines[idx].ast_skip_lines = 0;
    ctx->lines_count++;
    ctx->lines_mem_used += (len + sizeof(BppProgramLine));

    if (runtime_strstr(text, "NAMESPACE") != NULL || runtime_strstr(text, "namespace") != NULL || runtime_strstr(text, "Namespace") != NULL) {
        ctx->has_namespaces = true;
    }

    return true;
}

bool mem_program_delete(MemoryContext *ctx, BppLineNumber line) {
    if (!ctx) return false;
    size_t idx = 0;
    if (!find_line_index(ctx, line, &idx)) {
        return false; // Not found
    }

    // Free memory for this line
    HalContext *hal = hal_get();
    size_t len = runtime_strlen(ctx->lines[idx].text);
    if (hal && hal->mem.free) {
        hal->mem.free(ctx->lines[idx].text);
    }
    if (ctx->lines[idx].ast_cache) {
        eval_ast_free_tree(ctx->lines[idx].ast_cache);
        ctx->lines[idx].ast_cache = NULL;
        ctx->lines[idx].ast_valid = false;
    }
    ctx->lines_mem_used -= (len + sizeof(BppProgramLine));

    // Shift remaining lines
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

bool mem_program_find_line_index(MemoryContext *ctx, BppLineNumber line, size_t *out_idx) {
    if (!ctx || !out_idx) return false;
    return find_line_index(ctx, line, out_idx);
}

bool mem_program_has_namespaces(MemoryContext *ctx) {
    return ctx ? ctx->has_namespaces : false;
}

void mem_program_set_has_namespaces(MemoryContext *ctx, bool has_ns) {
    if (ctx) {
        ctx->has_namespaces = has_ns;
    }
}

void mem_program_clear(MemoryContext *ctx) {
    if (!ctx) return;
    HalContext *hal = hal_get();
    for (size_t i = 0; i < ctx->lines_count; ++i) {
        if (ctx->lines[i].text && hal && hal->mem.free) {
            hal->mem.free(ctx->lines[i].text);
        }
        if (ctx->lines[i].ast_cache) {
            eval_ast_free_tree(ctx->lines[i].ast_cache);
            ctx->lines[i].ast_cache = NULL;
            ctx->lines[i].ast_valid = false;
        }
    }
    ctx->lines_count = 0;
    ctx->lines_mem_used = 0;
    ctx->program_version[0] = '\0';
    ctx->has_namespaces = false;
}

void mem_program_set_version(MemoryContext *ctx, const char *ver_str) {
    if (!ctx) return;
    if (!ver_str) {
        ctx->program_version[0] = '\0';
        return;
    }
    runtime_strncpy(ctx->program_version, ver_str, sizeof(ctx->program_version) - 1);
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
        return NULL; // String heap limit
    }
    HalContext *hal = hal_get();
    size_t *ptr = NULL;
    if (hal && hal->mem.alloc) {
        ptr = (size_t *)hal->mem.alloc(total_size);
    }
    if (!ptr) return NULL;
    *ptr = total_size;
    ctx->str_used += total_size;
    return (void *)(ptr + 1);
}

void mem_string_free(MemoryContext *ctx, void *ptr) {
    if (!ctx || !ptr) return;
    size_t *prefix = (size_t *)((char *)ptr - sizeof(size_t));
    size_t size = *prefix;
    ctx->str_used -= size;
    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(prefix);
    }
}

size_t mem_get_free_ram(MemoryContext *ctx) {
    if (!ctx) return 0;
    // Dynamic memory left across variables, strings, scratch, and programs
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
        runtime_snprintf(buf, buf_size, "%lu Bytes", (unsigned long)bytes);
    } else {
        runtime_snprintf(buf, buf_size, "%.2f %s", size, units[idx]);
    }
}

bool mem_lib_program_insert(MemoryContext *ctx, BppLineNumber line, const char *text) {
    if (!ctx || !text) return false;
    HalContext *hal = hal_get();
    size_t len = runtime_strlen(text);

    if (ctx->lib_lines_count >= ctx->lib_lines_capacity) {
        size_t old_cap = ctx->lib_lines_capacity;
        size_t new_cap = (ctx->lib_lines_capacity == 0) ? 16 : (ctx->lib_lines_capacity * 2);
        BppProgramLine *new_lines = NULL;
        if (hal && hal->mem.realloc) {
            new_lines = (BppProgramLine *)hal->mem.realloc(ctx->lib_lines, new_cap * sizeof(BppProgramLine));
        } else if (hal && hal->mem.alloc) {
            new_lines = (BppProgramLine *)hal->mem.alloc(new_cap * sizeof(BppProgramLine));
            if (new_lines && ctx->lib_lines) {
                runtime_memcpy(new_lines, ctx->lib_lines, old_cap * sizeof(BppProgramLine));
                if (hal->mem.free) hal->mem.free(ctx->lib_lines);
            }
        }
        if (!new_lines) return false;
        ctx->lib_lines = new_lines;
        ctx->lib_lines_capacity = new_cap;
    }

    char *text_copy = NULL;
    if (hal && hal->mem.alloc) {
        text_copy = (char *)hal->mem.alloc(len + 1);
    }
    if (!text_copy) return false;
    runtime_strcpy(text_copy, text);

    ctx->lib_lines[ctx->lib_lines_count].line_number = line;
    ctx->lib_lines[ctx->lib_lines_count].text = text_copy;
    ctx->lib_lines[ctx->lib_lines_count].ast_cache = NULL;
    ctx->lib_lines[ctx->lib_lines_count].ast_valid = false;
    ctx->lib_lines[ctx->lib_lines_count].ast_skip_lines = 0;
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
    HalContext *hal = hal_get();
    for (size_t i = 0; i < ctx->lib_lines_count; ++i) {
        if (ctx->lib_lines[i].text && hal && hal->mem.free) {
            hal->mem.free(ctx->lib_lines[i].text);
        }
    }
    ctx->lib_lines_count = 0;
}


