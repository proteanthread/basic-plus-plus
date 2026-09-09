// FILENAME: var_scope.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for var_scope within BASIC++.
//
// ---- Includes ----

#include "runtime/variables_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/memory/alloc.h"
#include "runtime/math/math.h"

//
// ---- Scopes and Namespaces ----

static void clear_scope_defs(VariableContext *ctx) {
    ScopeDefMapping *curr = ctx->scope_defs;
    while (curr) {
        ScopeDefMapping *next = curr->next;
        runtime_free(curr);
        curr = next;
    }
    ctx->scope_defs = NULL;
}

void var_clear_scope(VariableContext *ctx, const char *prefix) {
    if (!ctx || !prefix || prefix[0] == '\0') return;

    char norm_prefix[256];
    size_t plen = 0;
    while (prefix[plen] && plen < sizeof(norm_prefix) - 1) {
        norm_prefix[plen] = (char)runtime_toupper((unsigned char)prefix[plen]);
        plen++;
    }
    norm_prefix[plen] = '\0';

    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *prev = NULL;
        VarEntry *curr = ctx->buckets[i];
        while (curr) {
            if (runtime_strncmp(curr->name, norm_prefix, plen) == 0) {
                VarEntry *temp = curr;
                if (prev) {
                    prev->next = curr->next;
                } else {
                    ctx->buckets[i] = curr->next;
                }
                curr = curr->next;

                if ((temp->value.type == VAL_STRING || temp->value.type == VAL_ARRAY_REF) && temp->value.as.string) {
                    str_release(ctx->str, temp->value.as.string);
                } else if (temp->value.type == VAL_MAP && temp->value.as.map) {
                    map_release(ctx->str, temp->value.as.map);
                } else if (temp->value.type == VAL_SET && temp->value.as.set) {
                    set_release(ctx->str, temp->value.as.set);
                } else if (temp->value.type == VAL_GROUP && temp->value.as.group) {
                    group_release(ctx->str, temp->value.as.group);
                }
                runtime_free(temp->name);
                runtime_free(temp);
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }
    runtime_memset(ctx->direct_cache, 0, sizeof(ctx->direct_cache));
    runtime_memset(ctx->fast_scalars_valid, 0, sizeof(ctx->fast_scalars_valid));
}

void var_set_scope(VariableContext *ctx, const char *scope) {
    if (!ctx) return;
    if (scope) {
        runtime_strncpy(ctx->active_scope, scope, sizeof(ctx->active_scope) - 1);
        ctx->active_scope[sizeof(ctx->active_scope) - 1] = '\0';
    } else {
        ctx->active_scope[0] = '\0';
    }
    runtime_memset(ctx->direct_cache, 0, sizeof(ctx->direct_cache));
    runtime_memset(ctx->fast_scalars_valid, 0, sizeof(ctx->fast_scalars_valid));
}

void var_set_shared(VariableContext *ctx, const char *name) {
    if (!ctx || !name || name[0] == '\0') return;
    if (ctx && ctx->shared_count < 64) {
        normalize_name(ctx, ctx->shared_vars[ctx->shared_count], name, 64);
        ctx->shared_count++;
    }
}

void var_set_namespace(VariableContext *ctx, const char *ns) {
    if (!ctx) return;
    if (ns) {
        runtime_strncpy(ctx->active_namespace, ns, sizeof(ctx->active_namespace) - 1);
        ctx->active_namespace[sizeof(ctx->active_namespace) - 1] = '\0';
    } else {
        ctx->active_namespace[0] = '\0';
    }
}

void var_set_explicit(VariableContext *ctx, bool enable) {
    if (ctx) {
        ctx->is_explicit = enable;
    }
}

bool var_is_explicit(VariableContext *ctx) {
    return ctx ? ctx->is_explicit : false;
}

void var_set_case_sensitive(VariableContext *ctx, bool enable) {
    if (ctx) {
        ctx->case_sensitive = enable;
    }
}

//
// ---- Default Types (DEFINT, DEFSTR, etc.) ----

static char get_base_letter(const char *name) {
    const char *p = runtime_strchr(name, ':');
    if (p) p++; else p = name;
    const char *dot = runtime_strchr(p, '.');
    if (dot) p = dot + 1;
    while (*p && !runtime_isalpha((unsigned char)*p)) p++;
    if (*p) return (char)runtime_toupper((unsigned char)*p);
    return '\0';
}

void var_set_def_type(VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type) {
    if (!ctx) return;
    char start = (char)runtime_toupper((unsigned char)start_letter);
    char end = (char)runtime_toupper((unsigned char)end_letter);
    if (start < 'A' || start > 'Z') start = 'A';
    if (end < 'A' || end > 'Z') end = 'Z';
    if (start > end) {
        char temp = start;
        start = end;
        end = temp;
    }

    if (scope && scope[0] != '\0') {
        ScopeDefMapping *curr = ctx->scope_defs;
        while (curr) {
            if (runtime_strcmp(curr->scope_name, scope) == 0) {
                break;
            }
            curr = curr->next;
        }
        if (!curr) {
            curr = (ScopeDefMapping *)runtime_calloc(1, sizeof(ScopeDefMapping));
            if (!curr) return;
            runtime_strncpy(curr->scope_name, scope, sizeof(curr->scope_name) - 1);
            curr->scope_name[sizeof(curr->scope_name) - 1] = '\0';
            for (int i = 0; i < 26; ++i) {
                curr->def_types[i] = ctx->global_def_types[i];
            }
            curr->next = ctx->scope_defs;
            ctx->scope_defs = curr;
        }
        if (curr) {
            for (char c = start; c <= end; ++c) {
                curr->def_types[c - 'A'] = type;
            }
        }
    } else {
        for (char c = start; c <= end; ++c) {
            int idx = c - 'A';
            ctx->global_def_types[idx] = type;
            ctx->fast_scalars_valid[idx] = false;
            ctx->fast_scalars[idx].type = type;
            if (type == VAL_INTEGER) {
                ctx->fast_scalars[idx].as.number = (double)((int32_t)ctx->fast_scalars[idx].as.number);
            }
        }
        for (int i = 0; i < DIRECT_VAR_CACHE_SIZE; ++i) {
            ctx->direct_cache[i].valid = false;
        }
    }

    for (int b = 0; b < HASH_BUCKETS; ++b) {
        VarEntry *entry = ctx->buckets[b];
        while (entry) {
            size_t nlen = runtime_strlen(entry->name);
            if (nlen > 0) {
                char last_ch = entry->name[nlen - 1];
                if (last_ch != '$' && last_ch != '%' && last_ch != '&' && last_ch != '!' && last_ch != '#') {
                    char bl = get_base_letter(entry->name);
                    if (bl >= start && bl <= end) {
                        entry->value.type = type;
                        if (type == VAL_INTEGER) {
                            entry->value.as.number = (double)((int32_t)entry->value.as.number);
                        }
                    }
                }
            }
            entry = entry->next;
        }
    }
}

ValueType var_get_def_type(VariableContext *ctx, const char *scope, char letter) {
    if (!ctx) return VAL_NUMBER;
    char l = (char)runtime_toupper((unsigned char)letter);
    if (l < 'A' || l > 'Z') return VAL_NUMBER;
    int idx = l - 'A';

    if (scope && scope[0] != '\0') {
        ScopeDefMapping *curr = ctx->scope_defs;
        while (curr) {
            if (runtime_strcmp(curr->scope_name, scope) == 0) {
                return curr->def_types[idx];
            }
            curr = curr->next;
        }
    }
    return ctx->global_def_types[idx];
}

//
// ---- COMMON Variables ----

void var_mark_common(VariableContext *ctx, const char *name) {
    if (!ctx || !name || !*name) return;
    char norm[64];
    normalize_name(ctx, norm, name, sizeof(norm));
    for (int i = 0; i < ctx->common_count; ++i) {
        if (runtime_strcmp(ctx->common_vars[i], norm) == 0) {
            return;
        }
    }
    if (ctx->common_count < 128) {
        runtime_snprintf(ctx->common_vars[ctx->common_count++], 64, "%s", norm);
    }
}

bool var_is_common(VariableContext *ctx, const char *name) {
    if (!ctx || !name || !*name) return false;
    char norm[64];
    normalize_name(ctx, norm, name, sizeof(norm));

    const char *base = runtime_strchr(norm, ':');
    if (base) {
        base++;
    } else {
        base = norm;
    }
    const char *dot = runtime_strchr(base, '.');
    if (dot) {
        base = dot + 1;
    }

    for (int i = 0; i < ctx->common_count; ++i) {
        if (runtime_strcmp(ctx->common_vars[i], base) == 0) {
            return true;
        }
    }
    return false;
}

void var_clear_for_chain(VariableContext *ctx) {
    if (!ctx) return;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *prev = NULL;
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            VarEntry *next = entry->next;
            if (var_is_common(ctx, entry->name)) {
                prev = entry;
            } else {
                if (prev) {
                    prev->next = next;
                } else {
                    ctx->buckets[i] = next;
                }
                runtime_free(entry->name);
                if ((entry->value.type == VAL_STRING || entry->value.type == VAL_ARRAY_REF) && entry->value.as.string) {
                    str_release(ctx->str, entry->value.as.string);
                } else if (entry->value.type == VAL_MAP && entry->value.as.map) {
                    map_release(ctx->str, entry->value.as.map);
                } else if (entry->value.type == VAL_SET && entry->value.as.set) {
                    set_release(ctx->str, entry->value.as.set);
                } else if (entry->value.type == VAL_GROUP && entry->value.as.group) {
                    group_release(ctx->str, entry->value.as.group);
                }
                runtime_free(entry);
            }
            entry = next;
        }
    }
    ctx->is_explicit = false;
    ctx->shared_count = 0;
    clear_scope_defs(ctx);
    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
    runtime_memset(ctx->direct_cache, 0, sizeof(ctx->direct_cache));
    runtime_memset(ctx->fast_scalars_valid, 0, sizeof(ctx->fast_scalars_valid));
}

//
// ---- Printing & Serialization ----

void var_print_all(VariableContext *ctx, void *vdev_ptr) {
    if (!ctx) return;
    VDevContext *vdev = (VDevContext *)vdev_ptr;

    if (vdev) {
        vdev_printf(vdev, "--- Active Variables ---\n");
    }
    log_info("--- Active Variables ---");

    int count = 0;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            char val_buf[256] = {0};
            if (entry->value.type == VAL_NUMBER) {
                num_format_display(val_buf, sizeof(val_buf), entry->value.as.number, false, false);
            } else if (entry->value.type == VAL_STRING) {
                if (entry->value.as.string) {
                    runtime_snprintf(val_buf, sizeof(val_buf), "\"%s\"", str_data(entry->value.as.string));
                } else {
                    runtime_snprintf(val_buf, sizeof(val_buf), "\"\"");
                }
            } else if (entry->value.type == VAL_INTEGER) {
                runtime_snprintf(val_buf, sizeof(val_buf), "%d", (int)entry->value.as.number);
            } else if (entry->value.type == VAL_COMPLEX) {
                char r_buf[64], i_buf[64];
                num_format_display(r_buf, sizeof(r_buf), entry->value.as.complex_val.real, false, false);
                num_format_display(i_buf, sizeof(i_buf), runtime_fabs(entry->value.as.complex_val.imag), false, false);
                if (entry->value.as.complex_val.imag < 0) {
                    runtime_snprintf(val_buf, sizeof(val_buf), "%s-%sI", r_buf, i_buf);
                } else {
                    runtime_snprintf(val_buf, sizeof(val_buf), "%s+%sI", r_buf, i_buf);
                }
            } else {
                runtime_snprintf(val_buf, sizeof(val_buf), "<unhandled type>");
            }

            if (vdev) {
                vdev_printf(vdev, "  %s = %s\n", entry->name, val_buf);
            }
            log_info("  %s = %s", entry->name, val_buf);
            count++;
            entry = entry->next;
        }
    }

    if (vdev) {
        vdev_printf(vdev, "Total variables: %d\n", count);
    }
    log_info("Total variables: %d", count);
}

bool var_serialize(VariableContext *ctx, void *fp) {
    if (!ctx || !fp) return false;

    uint32_t count = 0;
    for (int i = 0; i < HASH_BUCKETS; i++) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }

    platform_file_write(fp, &count, sizeof(count));

    for (int i = 0; i < HASH_BUCKETS; i++) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            uint32_t name_len = (uint32_t)runtime_strlen(entry->name);
            platform_file_write(fp, &name_len, sizeof(name_len));
            platform_file_write(fp, entry->name, name_len);

            uint32_t type = (uint32_t)entry->value.type;
            platform_file_write(fp, &type, sizeof(type));

            if (entry->value.type == VAL_NUMBER || entry->value.type == VAL_INTEGER) {
                platform_file_write(fp, &entry->value.as.number, sizeof(double));
            } else if (entry->value.type == VAL_STRING) {
                const char *s = str_data(entry->value.as.string);
                uint32_t s_len = s ? (uint32_t)runtime_strlen(s) : 0;
                platform_file_write(fp, &s_len, sizeof(s_len));
                if (s_len > 0) {
                    platform_file_write(fp, s, s_len);
                }
            } else if (entry->value.type == VAL_ARRAY_REF) {
                uint32_t a_len = entry->value.as.array_name ? (uint32_t)runtime_strlen(entry->value.as.array_name) : 0;
                platform_file_write(fp, &a_len, sizeof(a_len));
                if (a_len > 0) {
                    platform_file_write(fp, entry->value.as.array_name, a_len);
                }
            } else if (entry->value.type == VAL_FIELD_STRING) {
                platform_file_write(fp, &entry->value.as.field_str.channel, sizeof(int));
                platform_file_write(fp, &entry->value.as.field_str.offset, sizeof(int));
                platform_file_write(fp, &entry->value.as.field_str.length, sizeof(int));
            }
            entry = entry->next;
        }
    }
    return true;
}

bool var_deserialize(VariableContext *ctx, void *fp) {
    if (!ctx || !fp) return false;

    var_clear_all(ctx);

    uint32_t count = 0;
    if (platform_file_read(fp, &count, sizeof(count)) != sizeof(count)) return false;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t name_len = 0;
        if (platform_file_read(fp, &name_len, sizeof(name_len)) != sizeof(name_len)) return false;
        if (name_len == 0 || name_len > 1024) return false;

        char *name = (char *)runtime_calloc(1, (size_t)name_len + 1);
        if (!name) return false;
        if (platform_file_read(fp, name, name_len) != name_len) {
            runtime_free(name);
            return false;
        }
        name[name_len] = '\0';

        uint32_t type = 0;
        if (platform_file_read(fp, &type, sizeof(type)) != sizeof(type)) {
            runtime_free(name);
            return false;
        }

        BValue val;
        runtime_memset(&val, 0, sizeof(val));
        val.type = (ValueType)type;

        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            if (platform_file_read(fp, &val.as.number, sizeof(double)) != sizeof(double)) {
                runtime_free(name);
                return false;
            }
        } else if (val.type == VAL_STRING) {
            uint32_t s_len = 0;
            if (platform_file_read(fp, &s_len, sizeof(s_len)) != sizeof(s_len)) {
                runtime_free(name);
                return false;
            }
            if (s_len > 0) {
                char *s_buf = (char *)runtime_calloc(1, s_len + 1);
                if (!s_buf) { runtime_free(name); return false; }
                if (platform_file_read(fp, s_buf, s_len) != s_len) {
                    runtime_free(s_buf);
                    runtime_free(name);
                    return false;
                }
                s_buf[s_len] = '\0';
                val.as.string = str_create(ctx->str, s_buf, s_len);
                runtime_free(s_buf);
            } else {
                val.as.string = NULL;
            }
        } else if (val.type == VAL_ARRAY_REF) {
            uint32_t a_len = 0;
            if (platform_file_read(fp, &a_len, sizeof(a_len)) != sizeof(a_len)) {
                runtime_free(name);
                return false;
            }
            if (a_len > 0) {
                char *a_buf = (char *)runtime_calloc(1, a_len + 1);
                if (!a_buf) { runtime_free(name); return false; }
                if (platform_file_read(fp, a_buf, a_len) != a_len) {
                    runtime_free(a_buf);
                    runtime_free(name);
                    return false;
                }
                a_buf[a_len] = '\0';
                val.as.array_name = basic_strdup(a_buf);
                runtime_free(a_buf);
            } else {
                val.as.array_name = NULL;
            }
        } else if (val.type == VAL_FIELD_STRING) {
            if (platform_file_read(fp, &val.as.field_str.channel, sizeof(int)) != sizeof(int) ||
                platform_file_read(fp, &val.as.field_str.offset, sizeof(int)) != sizeof(int) ||
                platform_file_read(fp, &val.as.field_str.length, sizeof(int)) != sizeof(int)) {
                runtime_free(name);
                return false;
            }
        }

        VarEntry *new_entry = (VarEntry *)runtime_calloc(1, sizeof(VarEntry));
        if (!new_entry) {
            if (val.type == VAL_ARRAY_REF && val.as.array_name) runtime_free((void *)val.as.array_name);
            if (val.type == VAL_STRING && val.as.string) str_release(ctx->str, val.as.string);
            runtime_free(name);
            return false;
        }
        new_entry->name = name;
        new_entry->value = val;

        unsigned int bucket = hash_name(ctx, name);
        new_entry->next = ctx->buckets[bucket];
        ctx->buckets[bucket] = new_entry;
    }
    return true;
}
