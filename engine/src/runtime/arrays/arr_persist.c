// FILENAME: arr_persist.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays_internal.h)
// Provides core logic and interface definitions for arr_persist within BASIC++.
//
// ---- Includes ----

#include "runtime/arrays_internal.h"

//
// ---- Serialization and Deserialization ----

static bool arr_file_write(void *fp, const void *buf, size_t size) {
    if (!fp || !buf || size == 0) return true;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_write) return false;
    IoHandle handle = (IoHandle)(uintptr_t)fp;
    return (hal->io.file_write(handle, buf, 1, size) == size);
}

static bool arr_file_read(void *fp, void *buf, size_t size) {
    if (!fp || !buf || size == 0) return true;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_read) return false;
    IoHandle handle = (IoHandle)(uintptr_t)fp;
    return (hal->io.file_read(handle, buf, 1, size) == size);
}


bool arr_serialize(ArrayContext *ctx, void *fp) {
    if (!ctx || !fp) return false;

    uint32_t count = 0;
    for (int i = 0; i < HASH_BUCKETS; i++) {
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }

    if (!arr_file_write(fp, &count, sizeof(count))) return false;
    if (!arr_file_write(fp, &ctx->option_base, sizeof(ctx->option_base))) return false;
    if (!arr_file_write(fp, &ctx->last_det, sizeof(ctx->last_det))) return false;

    for (int i = 0; i < HASH_BUCKETS; i++) {
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            uint32_t name_len = (uint32_t)runtime_strlen(entry->name);
            if (!arr_file_write(fp, &name_len, sizeof(name_len))) return false;
            if (!arr_file_write(fp, entry->name, name_len)) return false;

            uint32_t type = (uint32_t)entry->type;
            if (!arr_file_write(fp, &type, sizeof(type))) return false;

            if (!arr_file_write(fp, &entry->num_dims, sizeof(entry->num_dims))) return false;
            if (!arr_file_write(fp, entry->bounds, sizeof(int) * 4)) return false;

            if (!arr_file_write(fp, &entry->total_size, sizeof(entry->total_size))) return false;

            for (int e = 0; e < entry->total_size; e++) {
                BValue *val = &entry->elements[e];
                uint32_t val_type = (uint32_t)val->type;
                if (!arr_file_write(fp, &val_type, sizeof(val_type))) return false;

                if (val->type == VAL_NUMBER || val->type == VAL_INTEGER) {
                    if (!arr_file_write(fp, &val->as.number, sizeof(double))) return false;
                } else if (val->type == VAL_STRING) {
                    const char *s = str_data(val->as.string);
                    uint32_t s_len = s ? (uint32_t)runtime_strlen(s) : 0;
                    if (!arr_file_write(fp, &s_len, sizeof(s_len))) return false;
                    if (s_len > 0) {
                        if (!arr_file_write(fp, s, s_len)) return false;
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
    HalContext *hal = hal_get();

    arr_clear_all(ctx);

    uint32_t count = 0;
    if (!arr_file_read(fp, &count, sizeof(count))) return false;
    if (!arr_file_read(fp, &ctx->option_base, sizeof(ctx->option_base))) return false;
    if (!arr_file_read(fp, &ctx->last_det, sizeof(ctx->last_det))) return false;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t name_len = 0;
        if (!arr_file_read(fp, &name_len, sizeof(name_len))) return false;

        char *name = NULL;
        if (hal && hal->mem.alloc) {
            name = (char *)hal->mem.alloc(name_len + 1);
        }
        if (!name) return false;
        if (!arr_file_read(fp, name, name_len)) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }
        name[name_len] = '\0';

        uint32_t type = 0;
        if (!arr_file_read(fp, &type, sizeof(type))) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }

        int num_dims = 0;
        if (!arr_file_read(fp, &num_dims, sizeof(num_dims))) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }

        int bounds[4];
        if (!arr_file_read(fp, bounds, sizeof(int) * 4)) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }

        int total_size = 0;
        if (!arr_file_read(fp, &total_size, sizeof(total_size))) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }
        if (total_size <= 0 || (size_t)total_size > SIZE_MAX / sizeof(BValue)) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }

        BValue *elements = NULL;
        if (hal && hal->mem.alloc) {
            elements = (BValue *)hal->mem.alloc((size_t)total_size * sizeof(BValue));
        }
        if (!elements) {
            if (hal && hal->mem.free) hal->mem.free(name);
            return false;
        }
        runtime_memset(elements, 0, (size_t)total_size * sizeof(BValue));

        for (int e = 0; e < total_size; e++) {
            uint32_t val_type = 0;
            if (!arr_file_read(fp, &val_type, sizeof(val_type))) {
                for (int k = 0; k < e; k++) {
                    if (elements[k].type == VAL_STRING && elements[k].as.string)
                        str_release(ctx->str, elements[k].as.string);
                }
                if (hal && hal->mem.free) {
                    hal->mem.free(elements);
                    hal->mem.free(name);
                }
                return false;
            }
            elements[e].type = (ValueType)val_type;

            if (elements[e].type == VAL_NUMBER || elements[e].type == VAL_INTEGER) {
                if (!arr_file_read(fp, &elements[e].as.number, sizeof(double))) {
                    for (int k = 0; k < e; k++) {
                        if (elements[k].type == VAL_STRING && elements[k].as.string)
                            str_release(ctx->str, elements[k].as.string);
                    }
                    if (hal && hal->mem.free) {
                        hal->mem.free(elements);
                        hal->mem.free(name);
                    }
                    return false;
                }
            } else if (elements[e].type == VAL_STRING) {
                uint32_t s_len = 0;
                if (!arr_file_read(fp, &s_len, sizeof(s_len))) {
                    for (int k = 0; k < e; k++) {
                        if (elements[k].type == VAL_STRING && elements[k].as.string)
                            str_release(ctx->str, elements[k].as.string);
                    }
                    if (hal && hal->mem.free) {
                        hal->mem.free(elements);
                        hal->mem.free(name);
                    }
                    return false;
                }
                if (s_len > 0) {
                    char *s_buf = NULL;
                    if (hal && hal->mem.alloc) {
                        s_buf = (char *)hal->mem.alloc(s_len + 1);
                    }
                    if (!s_buf) {
                        if (hal && hal->mem.free) {
                            hal->mem.free(elements);
                            hal->mem.free(name);
                        }
                        return false;
                    }
                    if (!arr_file_read(fp, s_buf, s_len)) {
                        if (hal && hal->mem.free) hal->mem.free(s_buf);
                        for (int k = 0; k < e; k++) {
                            if (elements[k].type == VAL_STRING && elements[k].as.string)
                                str_release(ctx->str, elements[k].as.string);
                        }
                        if (hal && hal->mem.free) {
                            hal->mem.free(elements);
                            hal->mem.free(name);
                        }
                        return false;
                    }
                    s_buf[s_len] = '\0';
                    elements[e].as.string = str_create(ctx->str, s_buf, s_len);
                    if (hal && hal->mem.free) hal->mem.free(s_buf);
                } else {
                    elements[e].as.string = NULL;
                }
            }
        }

        ArrayEntry *entry = NULL;
        if (hal && hal->mem.alloc) {
            entry = (ArrayEntry *)hal->mem.alloc(sizeof(ArrayEntry));
        }
        if (!entry) {
            if (hal && hal->mem.free) {
                hal->mem.free(elements);
                hal->mem.free(name);
            }
            return false;
        }
        runtime_memset(entry, 0, sizeof(ArrayEntry));
        entry->name = name;
        entry->type = (ValueType)type;
        entry->num_dims = num_dims;
        runtime_memcpy(entry->bounds, bounds, sizeof(bounds));
        entry->elements = elements;
        entry->total_size = total_size;

        unsigned int bucket = hash_name(name);
        entry->next = ctx->buckets[bucket];
        ctx->buckets[bucket] = entry;
    }
    return true;
}

//
// ---- Chaining Persistence and Aliasing ----

void arr_clear_for_chain(ArrayContext *ctx, VariableContext *var_ctx) {
    if (!ctx) return;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        ArrayEntry *prev = NULL;
        ArrayEntry *entry = ctx->buckets[i];
        while (entry) {
            ArrayEntry *next = entry->next;
            if (var_ctx && var_is_common(var_ctx, entry->name)) {
                prev = entry;
            } else {
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

bool arr_create_alias(ArrayContext *ctx, const char *alias_name, const char *target_name) {
    if (!ctx || !alias_name || !target_name) return false;
    HalContext *hal = hal_get();
    char norm_target[64];
    normalize_name(norm_target, target_name, sizeof(norm_target));
    unsigned int t_bucket = hash_name(norm_target);
    ArrayEntry *target = NULL;
    for (ArrayEntry *e = ctx->buckets[t_bucket]; e; e = e->next) {
        if (runtime_strcasecmp(e->name, norm_target) == 0) {
            target = e;
            break;
        }
    }
    if (!target) return false;

    char norm_alias[64];
    normalize_name(norm_alias, alias_name, sizeof(norm_alias));
    unsigned int a_bucket = hash_name(norm_alias);

    ArrayEntry *alias = NULL;
    if (hal && hal->mem.alloc) {
        alias = (ArrayEntry *)hal->mem.alloc(sizeof(ArrayEntry));
    }
    if (!alias) return false;
    runtime_memset(alias, 0, sizeof(ArrayEntry));

    size_t a_len = runtime_strlen(norm_alias);
    if (hal && hal->mem.alloc) {
        alias->name = (char *)hal->mem.alloc(a_len + 1);
    }
    if (!alias->name) {
        if (hal && hal->mem.free) hal->mem.free(alias);
        return false;
    }
    runtime_memcpy(alias->name, norm_alias, a_len + 1);

    alias->type = target->type;
    alias->num_dims = target->num_dims;
    runtime_memcpy(alias->bounds, target->bounds, sizeof(alias->bounds));
    alias->elements = target->elements;
    alias->total_size = target->total_size;
    alias->is_alias = true;
    alias->next = ctx->buckets[a_bucket];
    ctx->buckets[a_bucket] = alias;
    return true;
}

void arr_remove_alias(ArrayContext *ctx, const char *alias_name) {
    if (!ctx || !alias_name) return;
    char norm_alias[64];
    normalize_name(norm_alias, alias_name, sizeof(norm_alias));
    unsigned int a_bucket = hash_name(norm_alias);
    ArrayEntry *prev = NULL;
    ArrayEntry *e = ctx->buckets[a_bucket];
    while (e) {
        if (e->is_alias && runtime_strcasecmp(e->name, norm_alias) == 0) {
            if (prev) prev->next = e->next;
            else ctx->buckets[a_bucket] = e->next;
            free_entry(ctx, e);
            break;
        }
        prev = e;
        e = e->next;
    }
}

//
// ---- Virtual Array Channel Binding ----

BppError arr_dim_virtual(ArrayContext *ctx, const char *name, int num_dims, const int *bounds, int channel) {
    BppError err = arr_dim(ctx, name, num_dims, bounds);
    if (err.code == 0 && channel > 0) {
        arr_set_channel(ctx, name, channel);
    }
    return err;
}

int arr_get_channel(ArrayContext *ctx, const char *name) {
    if (!ctx || !name) return 0;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            return curr->channel;
        }
        curr = curr->next;
    }
    return 0;
}

void arr_set_channel(ArrayContext *ctx, const char *name, int channel) {
    if (!ctx || !name) return;
    char norm[256];
    normalize_name(norm, name, sizeof(norm));
    unsigned int bucket = hash_name(norm);
    ArrayEntry *curr = ctx->buckets[bucket];
    while (curr) {
        if (runtime_strcmp(curr->name, norm) == 0) {
            curr->channel = channel;
            return;
        }
        curr = curr->next;
    }
}

