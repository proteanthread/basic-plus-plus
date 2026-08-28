// FILENAME: arr_core.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays_internal.h)
// Provides core logic and interface definitions for arr_core within BASIC++.
//
// ---- Includes ----

#include "runtime/arrays_internal.h"

//
// ---- Entry Management and Lifecycle ----

void free_entry(ArrayContext *ctx, ArrayEntry *entry) {
    if (!entry) return;
    HalContext *hal = hal_get();
    if (!entry->is_alias && entry->elements) {
        for (int i = 0; i < entry->total_size; ++i) {
            if (entry->elements[i].type == VAL_STRING && entry->elements[i].as.string) {
                str_release(ctx->str, entry->elements[i].as.string);
            } else if (entry->elements[i].type == VAL_MAP && entry->elements[i].as.map) {
                map_release(ctx->str, entry->elements[i].as.map);
            }
        }
        if (hal && hal->mem.free) hal->mem.free(entry->elements);
    }
    if (entry->name && hal && hal->mem.free) hal->mem.free(entry->name);
    if (hal && hal->mem.free) hal->mem.free(entry);
}

ArrayContext *arr_init(MemoryContext *mem, StringContext *str) {
    if (!mem || !str) return NULL;
    HalContext *hal = hal_get();
    ArrayContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (ArrayContext *)hal->mem.alloc(sizeof(ArrayContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(ArrayContext));
    ctx->mem = mem;
    ctx->str = str;
    ctx->option_base = 0;
    ctx->last_det = 0.0;
    return ctx;
}

void arr_shutdown(ArrayContext *ctx) {
    if (!ctx) return;
    arr_clear_all(ctx);
    HalContext *hal = hal_get();
    if (hal && hal->mem.free) {
        hal->mem.free(ctx);
    }
}

void arr_clear_all(ArrayContext *ctx) {
    if (!ctx) return;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        ArrayEntry *curr = ctx->buckets[i];
        while (curr) {
            ArrayEntry *next = curr->next;
            free_entry(ctx, curr);
            curr = next;
        }
        ctx->buckets[i] = NULL;
    }
    runtime_memset(ctx->mru_cache, 0, sizeof(ctx->mru_cache));
    ctx->mru_head = 0;
}

//
// ---- Array Dimensioning and Deletion ----

BppError arr_dim(ArrayContext *ctx, const char *name, int num_dims, const int *bounds) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!ctx || !name || num_dims <= 0 || num_dims > 4) {
        err.code = 5;
        err.message = "Invalid array specifications";
        return err;
    }

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            err.code = 10;
            err.message = "Array already dimensioned";
            return err;
        }
        curr = curr->next;
    }

    size_t total = 1;
    for (int i = 0; i < num_dims; ++i) {
        if (bounds[i] < ctx->option_base) {
            err.code = 5;
            err.message = "Array bounds must be greater than or equal to option base";
            return err;
        }
        size_t dim_size = (size_t)(bounds[i] - ctx->option_base + 1);
        if (total > 0 && dim_size > SIZE_MAX / total) {
            err.code = 14;
            err.message = "Array size overflow";
            return err;
        }
        total *= dim_size;
    }

    HalContext *hal = hal_get();
    ArrayEntry *entry = NULL;
    if (hal && hal->mem.alloc) {
        entry = (ArrayEntry *)hal->mem.alloc(sizeof(ArrayEntry));
    }
    if (!entry) {
        err.code = 14;
        err.message = "Out of memory allocating array header";
        return err;
    }
    runtime_memset(entry, 0, sizeof(ArrayEntry));

    size_t n_len = runtime_strlen(norm);
    if (hal && hal->mem.alloc) {
        entry->name = (char *)hal->mem.alloc(n_len + 1);
    }
    if (!entry->name) {
        if (hal && hal->mem.free) hal->mem.free(entry);
        err.code = 14;
        err.message = "Out of memory allocating array name";
        return err;
    }
    runtime_memcpy(entry->name, norm, n_len + 1);

    size_t len = runtime_strlen(norm);
    char last = (len > 0) ? norm[len - 1] : '\0';
    if (last == '$') {
        entry->type = VAL_STRING;
    } else if (last == '%') {
        entry->type = VAL_INTEGER;
    } else {
        entry->type = VAL_NUMBER;
    }

    entry->num_dims = num_dims;
    runtime_memcpy(entry->bounds, bounds, num_dims * sizeof(int));
    entry->total_size = (int)total;

    if (hal && hal->mem.alloc) {
        entry->elements = (BValue *)hal->mem.alloc(total * sizeof(BValue));
    }
    if (!entry->elements) {
        if (hal && hal->mem.free) {
            hal->mem.free(entry->name);
            hal->mem.free(entry);
        }
        err.code = 14;
        err.message = "Out of memory allocating array elements";
        return err;
    }
    runtime_memset(entry->elements, 0, total * sizeof(BValue));

    for (size_t i = 0; i < total; ++i) {
        entry->elements[i].type = entry->type;
        if (entry->type == VAL_STRING) {
            entry->elements[i].as.string = NULL;
        } else {
            entry->elements[i].as.number = 0.0;
        }
    }

    entry->next = ctx->buckets[bucket];
    ctx->buckets[bucket] = entry;

    return err;
}

bool arr_erase(ArrayContext *ctx, const char *name) {
    if (!ctx || !name) return false;

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    ArrayEntry *prev = NULL;

    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                ctx->buckets[bucket] = curr->next;
            }
            free_entry(ctx, curr);
            return true;
        }
        prev = curr;
        curr = curr->next;
    }

    return false;
}

//
// ---- Attributes and Bounds Queries ----

int arr_ubound(ArrayContext *ctx, const char *name, int dimension, bool *out_found) {
    if (out_found) *out_found = false;
    if (!ctx || !name || dimension <= 0) return 0;

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];

    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            if (dimension > curr->num_dims) return 0;
            if (out_found) *out_found = true;
            return curr->bounds[dimension - 1];
        }
        curr = curr->next;
    }

    return 0;
}

void arr_set_option_base(ArrayContext *ctx, int base) {
    if (ctx && (base == 0 || base == 1)) {
        ctx->option_base = base;
    }
}

int arr_get_option_base(ArrayContext *ctx) {
    return ctx ? ctx->option_base : 0;
}

bool arr_exists(ArrayContext *ctx, const char *name) {
    if (!ctx || !name) return false;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) return true;
        curr = curr->next;
    }
    return false;
}

int arr_get_dimensions(ArrayContext *ctx, const char *name, int *out_bounds, int max_dims) {
    if (!ctx || !name || !out_bounds) return -1;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, norm) == 0) {
            int limit = (entry->num_dims < max_dims) ? entry->num_dims : max_dims;
            for (int i = 0; i < limit; i++) {
                out_bounds[i] = entry->bounds[i];
            }
            return entry->num_dims;
        }
        entry = entry->next;
    }
    return -1;
}

BValue *arr_get_flat_elements(ArrayContext *ctx, const char *name, int *out_total_size) {
    if (!ctx || !name) return NULL;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, norm) == 0) {
            if (out_total_size) *out_total_size = entry->total_size;
            return entry->elements;
        }
        entry = entry->next;
    }
    return NULL;
}

BValue *arr_ensure_capacity(ArrayContext *ctx, const char *name, int min_elements) {
    if (!ctx || !name) return NULL;
    HalContext *hal = hal_get();
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, norm) == 0) {
            if (entry->total_size < min_elements) {
                BValue *new_elems = NULL;
                if (hal && hal->mem.realloc) {
                    new_elems = (BValue *)hal->mem.realloc(entry->elements, min_elements * sizeof(BValue));
                } else if (hal && hal->mem.alloc) {
                    new_elems = (BValue *)hal->mem.alloc(min_elements * sizeof(BValue));
                    if (new_elems && entry->elements) {
                        runtime_memcpy(new_elems, entry->elements, entry->total_size * sizeof(BValue));
                        if (hal->mem.free) hal->mem.free(entry->elements);
                    }
                }
                if (new_elems) {
                    for (int i = entry->total_size; i < min_elements; i++) {
                        new_elems[i].type = VAL_NUMBER;
                        new_elems[i].as.number = 0.0;
                    }
                    entry->elements = new_elems;
                    entry->total_size = min_elements;
                }
            }
            return entry->elements;
        }
        entry = entry->next;
    }
    return NULL;
}

void arr_set_type(ArrayContext *ctx, const char *name, ValueType type) {
    if (!ctx || !name) return;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, norm) == 0) {
            entry->type = type;
            return;
        }
        entry = entry->next;
    }
}

double arr_get_last_det(ArrayContext *ctx) {
    return ctx ? ctx->last_det : 0.0;
}

void arr_set_last_det(ArrayContext *ctx, double val) {
    if (ctx) ctx->last_det = val;
}

