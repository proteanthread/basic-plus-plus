// FILENAME: arr_access.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays_internal.h)
// Provides core logic and interface definitions for arr_access within BASIC++.
//
// ---- Includes ----

#include "runtime/arrays_internal.h"

//
// ---- Index Calculation and Auto-Expansion ----

int get_flat_index(int option_base, int num_dims, const int *bounds, const int *indices) {
    if (!bounds || !indices || num_dims <= 0 || num_dims > 4) return -1;
    int flat = 0;
    int multiplier = 1;
    for (int i = num_dims - 1; i >= 0; i--) {
        if (indices[i] < option_base || indices[i] > bounds[i]) {
            return -1;
        }
        int dim_size = bounds[i] - option_base + 1;
        flat += (indices[i] - option_base) * multiplier;
        multiplier *= dim_size;
    }
    return flat;
}

bool auto_expand_array(ArrayContext *ctx, ArrayEntry *entry, const int *indices) {
    if (!ctx || !entry || !indices) return false;

    int new_bounds[4] = {0};
    bool expansion_needed = false;
    for (int i = 0; i < entry->num_dims; i++) {
        if (indices[i] < ctx->option_base) {
            return false;
        }
        if (indices[i] > entry->bounds[i]) {
            new_bounds[i] = indices[i];
            expansion_needed = true;
        } else {
            new_bounds[i] = entry->bounds[i];
        }
    }
    if (!expansion_needed) return true;

    size_t new_total = 1;
    for (int i = 0; i < entry->num_dims; i++) {
        size_t dim_size = (size_t)(new_bounds[i] - ctx->option_base + 1);
        if (new_total > 0 && dim_size > SIZE_MAX / new_total) {
            return false;
        }
        new_total *= dim_size;
    }

    HalContext *hal = hal_get();
    BValue *new_elements = NULL;
    if (hal && hal->mem.alloc) {
        new_elements = (BValue *)hal->mem.alloc(new_total * sizeof(BValue));
    }
    if (!new_elements) return false;
    runtime_memset(new_elements, 0, new_total * sizeof(BValue));

    for (size_t i = 0; i < new_total; i++) {
        new_elements[i].type = entry->type;
        if (entry->type == VAL_STRING) {
            new_elements[i].as.string = NULL;
        } else {
            new_elements[i].as.number = 0.0;
        }
    }

    for (int old_flat = 0; old_flat < entry->total_size; old_flat++) {
        int temp = old_flat;
        int coords[4] = {0};
        for (int i = entry->num_dims - 1; i >= 0; i--) {
            int dim_size = entry->bounds[i] - ctx->option_base + 1;
            coords[i] = ctx->option_base + (temp % dim_size);
            temp /= dim_size;
        }
        int new_flat = get_flat_index(ctx->option_base, entry->num_dims, new_bounds, coords);
        if (new_flat >= 0 && (size_t)new_flat < new_total) {
            new_elements[new_flat] = entry->elements[old_flat];
        }
    }

    if (hal && hal->mem.free) {
        hal->mem.free(entry->elements);
    }
    entry->elements = new_elements;
    entry->total_size = (int)new_total;
    runtime_memcpy(entry->bounds, new_bounds, entry->num_dims * sizeof(int));
    return true;
}

//
// ---- Element Access Entry Point ----

BValue *arr_get_element(ArrayContext *ctx, const char *name, int num_dims, const int *indices, BppError *err) {
    if (!ctx || !name || !indices || !err) return NULL;

    ArrayEntry *curr = NULL;
    for (int i = 0; i < MRU_ARRAY_CACHE_SIZE; ++i) {
        if (ctx->mru_cache[i].entry && runtime_strcasecmp(ctx->mru_cache[i].name, name) == 0) {
            curr = ctx->mru_cache[i].entry;
            break;
        }
    }

    char norm[256];
    if (!curr) {
        normalize_name(norm, name, sizeof(norm));
        unsigned int bucket = hash_name(norm);
        curr = ctx->buckets[bucket];

        while (curr) {
            if (runtime_strcmp(curr->name, norm) == 0) {
                int slot = ctx->mru_head;
                ctx->mru_head = (ctx->mru_head + 1) % MRU_ARRAY_CACHE_SIZE;
                runtime_strncpy(ctx->mru_cache[slot].name, name, sizeof(ctx->mru_cache[slot].name) - 1);
                ctx->mru_cache[slot].name[sizeof(ctx->mru_cache[slot].name) - 1] = '\0';
                ctx->mru_cache[slot].entry = curr;
                break;
            }
            curr = curr->next;
        }
    }

    if (curr) {
        if (curr->num_dims != num_dims) {
            err->code = 9;
            err->message = "Array dimension count mismatch";
            return NULL;
        }
        int flat = -1;
        if (num_dims == 1) {
            int idx0 = indices[0];
            if (idx0 >= ctx->option_base && idx0 <= curr->bounds[0]) {
                flat = idx0 - ctx->option_base;
            }
        } else if (num_dims == 2) {
            int idx0 = indices[0];
            int idx1 = indices[1];
            if (idx0 >= ctx->option_base && idx0 <= curr->bounds[0] &&
                idx1 >= ctx->option_base && idx1 <= curr->bounds[1]) {
                flat = (idx0 - ctx->option_base) * (curr->bounds[1] - ctx->option_base + 1) + (idx1 - ctx->option_base);
            }
        } else {
            flat = get_flat_index(ctx->option_base, curr->num_dims, curr->bounds, indices);
        }

        if (flat < 0 || flat >= curr->total_size) {
            bool valid_lower = true;
            for (int i = 0; i < num_dims; i++) {
                if (indices[i] < ctx->option_base) {
                    valid_lower = false;
                    break;
                }
            }
            if (valid_lower && auto_expand_array(ctx, curr, indices)) {
                flat = get_flat_index(ctx->option_base, curr->num_dims, curr->bounds, indices);
            } else {
                err->code = 9;
                err->message = "Array index out of range";
                return NULL;
            }
        }
        return &curr->elements[flat];
    }

    int def_bounds[4] = {0};
    for (int i = 0; i < num_dims; i++) {
        if (indices[i] < ctx->option_base) {
            err->code = 9;
            err->message = "Subscript below option base";
            return NULL;
        }
        def_bounds[i] = (indices[i] > 10) ? indices[i] : 10;
    }
    normalize_name(norm, name, sizeof(norm));
    BppError dim_err = arr_dim(ctx, norm, num_dims, def_bounds);
    if (dim_err.code != 0) {
        *err = dim_err;
        return NULL;
    }

    unsigned int bucket = hash_name(norm);
    curr = ctx->buckets[bucket];
    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            int flat = get_flat_index(ctx->option_base, curr->num_dims, curr->bounds, indices);
            if (flat >= 0 && flat < curr->total_size) {
                return &curr->elements[flat];
            }
        }
        curr = curr->next;
    }

    err->code = 9;
    err->message = "Array allocation failed";
    return NULL;
}

