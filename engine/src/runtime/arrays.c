/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file arrays.c
 * @brief Dynamic Array Management Subsystem implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements multi-dimensional array declaration, element lookup,
 *   erasure, bounds queries, and context lifecycles.
 * - Why it exists: Provides QBASIC/GW-BASIC compatible array storage for variables of different suffixes.
 * - Why it works this way: It uses an uppercase-normalized chained hash map of array nodes.
 *   Each node contains flat element stores on the heap, computed using dimension offset strides.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Suffix mapping rules, maximum bounds checks, default values.
 * - What cannot be changed: Memory leak cleanups on shutdown (must release ref-counted string elements).
 * - What to expect: Re-dimming an already existing array without calling ERASE returns error code 10.
 * - What to do if something breaks: Trace get_flat_index calculations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Index parameters fit within standard system limits.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add OPTION BASE 1 configurations dynamically.
 * - How to write external extensions: Plugins utilize arr_get_element to interact with array matrices.
 */

#include "runtime/arrays.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define HASH_BUCKETS 128

typedef struct ArrayEntry {
    char              *name;
    ValueType          type;
    int                num_dims;
    int                bounds[4];
    BValue            *elements;
    int                total_size;
    struct ArrayEntry *next;
} ArrayEntry;

struct ArrayContext {
    MemoryContext *mem;
    StringContext *str;
    ArrayEntry    *buckets[HASH_BUCKETS];
    int            option_base;
    double         last_det;
};

/* Case-insensitive hash helper */
static unsigned int hash_name(const char *name) {
    unsigned int hash = 5381;
    while (*name) {
        hash = ((hash << 5) + hash) + (unsigned int)toupper((unsigned char)*name);
        name++;
    }
    return hash % HASH_BUCKETS;
}

/* Helper to normalize name to upper case */
static void normalize_name(char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = (char)toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

/* Portable C17 strdup replacement */
static char *bpp_strdup(const char *src) {
    size_t len = strlen(src);
    char *dest = (char *)calloc(1, len + 1);
    if (dest) {
        memcpy(dest, src, len + 1);
    }
    return dest;
}

ArrayContext *arr_init(MemoryContext *mem, StringContext *str) {
    if (!mem || !str) return NULL;
    ArrayContext *ctx = (ArrayContext *)calloc(1, sizeof(ArrayContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->str = str;
    ctx->option_base = 0;
    ctx->last_det = 0.0;
    return ctx;
}

static void free_entry(ArrayContext *ctx, ArrayEntry *entry) {
    if (!entry) return;
    /* Release string references */
    if (entry->elements) {
        for (int i = 0; i < entry->total_size; ++i) {
            if (entry->elements[i].type == VAL_STRING && entry->elements[i].as.string) {
                str_release(ctx->str, entry->elements[i].as.string);
            } else if (entry->elements[i].type == VAL_MAP && entry->elements[i].as.map) {
                bpp_map_release(ctx->str, entry->elements[i].as.map);
            }
        }
        free(entry->elements);
    }
    free(entry->name);
    free(entry);
}

void arr_shutdown(ArrayContext *ctx) {
    if (!ctx) return;
    arr_clear_all(ctx);
    free(ctx);
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
}

BppError arr_dim(ArrayContext *ctx, const char *name, int num_dims, const int *bounds) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!ctx || !name || num_dims <= 0 || num_dims > 4) {
        err.code = 5; /* Illegal function call */
        err.message = "Invalid array specifications";
        return err;
    }

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    while (curr) {
        if (strcmp(curr->name, norm) == 0) {
            err.code = 10; /* Duplicate definition */
            err.message = "Array already dimensioned";
            return err;
        }
        curr = curr->next;
    }

    /* Compute total flat size */
    size_t total = 1;
    for (int i = 0; i < num_dims; ++i) {
        if (bounds[i] < ctx->option_base) {
            err.code = 5;
            err.message = "Array bounds must be greater than or equal to option base";
            return err;
        }
        size_t dim_size = (size_t)(bounds[i] - ctx->option_base + 1);
        if (total > 0 && dim_size > SIZE_MAX / total) {
            err.code = 14; err.message = "Array size overflow"; return err;
        }
        total *= dim_size;
    }

    /* Allocate entry */
    ArrayEntry *entry = (ArrayEntry *)calloc(1, sizeof(ArrayEntry));
    if (!entry) {
        err.code = 14; /* Out of memory */
        err.message = "Out of memory allocating array header";
        return err;
    }

    entry->name = bpp_strdup(norm);
    if (!entry->name) {
        free(entry);
        err.code = 14;
        err.message = "Out of memory allocating array name";
        return err;
    }

    /* Determine element type from suffix */
    size_t len = strlen(norm);
    char last = (len > 0) ? norm[len - 1] : '\0';
    if (last == '$') {
        entry->type = VAL_STRING;
    } else if (last == '%') {
        entry->type = VAL_INTEGER;
    } else {
        entry->type = VAL_NUMBER;
    }

    entry->num_dims = num_dims;
    memcpy(entry->bounds, bounds, num_dims * sizeof(int));
    entry->total_size = (int)total;

    /* Allocate element storage */
    entry->elements = (BValue *)calloc(total, sizeof(BValue));
    if (!entry->elements) {
        free(entry->name);
        free(entry);
        err.code = 14;
        err.message = "Out of memory allocating array elements";
        return err;
    }

    /* Initialize default element values */
    for (size_t i = 0; i < total; ++i) {
        entry->elements[i].type = entry->type;
        if (entry->type == VAL_STRING) {
            entry->elements[i].as.string = NULL;
        } else {
            entry->elements[i].as.number = 0.0;
        }
    }

    /* Insert into bucket chain */
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
        if (strcmp(curr->name, norm) == 0) {
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

static int get_flat_index(int option_base, int num_dims, const int *bounds, const int *indices) {
    int idx = 0;
    for (int i = 0; i < num_dims; ++i) {
        if (indices[i] < option_base || indices[i] > bounds[i]) {
            return -1; /* Out of bounds */
        }
        int dim_size = bounds[i] - option_base + 1;
        idx = idx * dim_size + (indices[i] - option_base);
    }
    return idx;
}

BValue *arr_get_element(ArrayContext *ctx, const char *name, int num_dims, const int *indices, BppError *err) {
    if (!ctx || !name || !indices || !err) return NULL;

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];

    while (curr) {
        if (strcmp(curr->name, norm) == 0) {
            if (curr->num_dims != num_dims) {
                err->code = 9; /* Subscript out of range */
                err->message = "Array dimension count mismatch";
                return NULL;
            }
            int flat = get_flat_index(ctx->option_base, curr->num_dims, curr->bounds, indices);
            if (flat < 0 || flat >= curr->total_size) {
                err->code = 9; /* Subscript out of range */
                err->message = "Array index out of range";
                return NULL;
            }
            return &curr->elements[flat];
        }
        curr = curr->next;
    }

    /* If not found, GW-BASIC dynamically creates a default 10-element array!
     * But in standard configurations, it's safer to report it's not dimensioned
     * unless we auto-allocate. Let's dynamically auto-dim to 10 elements per dimension
     * if the name exists, or return an error. Let's return error 9 for safety.
     */
    err->code = 9;
    err->message = "Array not dimensioned";
    return NULL;
}

int arr_ubound(ArrayContext *ctx, const char *name, int dimension, bool *out_found) {
    if (out_found) *out_found = false;
    if (!ctx || !name || dimension <= 0) return 0;

    char norm[256];
    normalize_name(norm, name, sizeof(norm));

    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];

    while (curr) {
        if (strcmp(curr->name, norm) == 0) {
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
        if (strcmp(curr->name, norm) == 0) return true;
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
        if (strcmp(entry->name, norm) == 0) {
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
        if (strcmp(entry->name, norm) == 0) {
            if (out_total_size) *out_total_size = entry->total_size;
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
        if (strcmp(entry->name, norm) == 0) {
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

bool arr_serialize(ArrayContext *ctx, void *fp) {
    if (!ctx || !fp) return false;
    FILE *f = (FILE *)fp;

    uint32_t count = 0;
    for (int i = 0; i < HASH_BUCKETS; i++) {
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }

    fwrite(&count, sizeof(count), 1, f);
    fwrite(&ctx->option_base, sizeof(ctx->option_base), 1, f);
    fwrite(&ctx->last_det, sizeof(ctx->last_det), 1, f);

    for (int i = 0; i < HASH_BUCKETS; i++) {
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            uint32_t name_len = (uint32_t)strlen(entry->name);
            fwrite(&name_len, sizeof(name_len), 1, f);
            fwrite(entry->name, 1, name_len, f);

            uint32_t type = (uint32_t)entry->type;
            fwrite(&type, sizeof(type), 1, f);

            fwrite(&entry->num_dims, sizeof(entry->num_dims), 1, f);
            fwrite(entry->bounds, sizeof(int), 4, f);

            fwrite(&entry->total_size, sizeof(entry->total_size), 1, f);

            for (int e = 0; e < entry->total_size; e++) {
                BValue *val = &entry->elements[e];
                uint32_t val_type = (uint32_t)val->type;
                fwrite(&val_type, sizeof(val_type), 1, f);

                if (val->type == VAL_NUMBER || val->type == VAL_INTEGER) {
                    fwrite(&val->as.number, sizeof(double), 1, f);
                } else if (val->type == VAL_STRING) {
                    const char *s = str_data(val->as.string);
                    uint32_t s_len = s ? (uint32_t)strlen(s) : 0;
                    fwrite(&s_len, sizeof(s_len), 1, f);
                    if (s_len > 0) {
                        fwrite(s, 1, s_len, f);
                    }
                }
            }
            entry = entry->next;
        }
    }
    return true;
}

bool arr_deserialize(ArrayContext *ctx, void *fp) {
    if (!ctx || !fp) return false;
    FILE *f = (FILE *)fp;

    arr_clear_all(ctx);

    uint32_t count = 0;
    if (fread(&count, sizeof(count), 1, f) != 1) return false;
    if (fread(&ctx->option_base, sizeof(ctx->option_base), 1, f) != 1) return false;
    if (fread(&ctx->last_det, sizeof(ctx->last_det), 1, f) != 1) return false;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t name_len = 0;
        if (fread(&name_len, sizeof(name_len), 1, f) != 1) return false;

        char *name = (char *)calloc(1, name_len + 1);
        if (!name) return false;
        if (fread(name, 1, name_len, f) != name_len) {
            free(name);
            return false;
        }
        name[name_len] = '\0';

        uint32_t type = 0;
        if (fread(&type, sizeof(type), 1, f) != 1) {
            free(name);
            return false;
        }

        int num_dims = 0;
        if (fread(&num_dims, sizeof(num_dims), 1, f) != 1) {
            free(name);
            return false;
        }

        int bounds[4];
        if (fread(bounds, sizeof(int), 4, f) != 4) {
            free(name);
            return false;
        }

        int total_size = 0;
        if (fread(&total_size, sizeof(total_size), 1, f) != 1) {
            free(name);
            return false;
        }
        if (total_size <= 0 || (size_t)total_size > SIZE_MAX / sizeof(BValue)) {
            free(name);
            return false;
        }

        BValue *elements = (BValue *)calloc(total_size, sizeof(BValue));
        if (!elements) {
            free(name);
            return false;
        }

        for (int e = 0; e < total_size; e++) {
            uint32_t val_type = 0;
            if (fread(&val_type, sizeof(val_type), 1, f) != 1) {
                for (int k = 0; k < e; k++) {
                    if (elements[k].type == VAL_STRING && elements[k].as.string)
                        str_release(ctx->str, elements[k].as.string);
                }
                free(elements);
                free(name);
                return false;
            }
            elements[e].type = (ValueType)val_type;

            if (elements[e].type == VAL_NUMBER || elements[e].type == VAL_INTEGER) {
                if (fread(&elements[e].as.number, sizeof(double), 1, f) != 1) {
                    for (int k = 0; k < e; k++) {
                        if (elements[k].type == VAL_STRING && elements[k].as.string)
                            str_release(ctx->str, elements[k].as.string);
                    }
                    free(elements);
                    free(name);
                    return false;
                }
            } else if (elements[e].type == VAL_STRING) {
                uint32_t s_len = 0;
                if (fread(&s_len, sizeof(s_len), 1, f) != 1) {
                    for (int k = 0; k < e; k++) {
                        if (elements[k].type == VAL_STRING && elements[k].as.string)
                            str_release(ctx->str, elements[k].as.string);
                    }
                    free(elements);
                    free(name);
                    return false;
                }
                if (s_len > 0) {
                    char *s_buf = (char *)calloc(1, s_len + 1);
                    if (!s_buf) { free(elements); free(name); return false; }
                    if (fread(s_buf, 1, s_len, f) != s_len) {
                        free(s_buf);
                        for (int k = 0; k < e; k++) {
                            if (elements[k].type == VAL_STRING && elements[k].as.string)
                                str_release(ctx->str, elements[k].as.string);
                        }
                        free(elements);
                        free(name);
                        return false;
                    }
                    s_buf[s_len] = '\0';
                    elements[e].as.string = str_create(ctx->str, s_buf, s_len);
                    free(s_buf);
                } else {
                    elements[e].as.string = NULL;
                }
            }
        }

        ArrayEntry *entry = (ArrayEntry *)calloc(1, sizeof(ArrayEntry));
        if (!entry) { free(elements); free(name); return false; }
        entry->name = name;
        entry->type = (ValueType)type;
        entry->num_dims = num_dims;
        memcpy(entry->bounds, bounds, sizeof(bounds));
        entry->elements = elements;
        entry->total_size = total_size;

        unsigned int bucket = hash_name(name);
        entry->next = ctx->buckets[bucket];
        ctx->buckets[bucket] = entry;
    }
    return true;
}

void arr_clear_for_chain(ArrayContext *ctx, VariableContext *var_ctx) {
    if (!ctx) return;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        ArrayEntry *prev = NULL;
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            ArrayEntry *next = entry->next;
            if (var_ctx && var_is_common(var_ctx, entry->name)) {
                /* Keep this array */
                prev = entry;
            } else {
                /* Free this array */
                if (prev) {
                    prev->next = next;
                } else {
                    ctx->buckets[i] = next;
                }
                free_entry(ctx, entry);
            }
            entry = next;
        }
    }
}


