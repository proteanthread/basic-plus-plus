// FILENAME: var_lookup.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for var_lookup within BASIC++.
//
// ---- Includes ----

#include "runtime/variables_internal.h"

//
// ---- String & Hash Helpers ----

int basic_strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = (unsigned char)*s1;
        int c2 = (unsigned char)*s2;
        if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
        if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    int c1 = (unsigned char)*s1;
    int c2 = (unsigned char)*s2;
    if (c1 >= 'a' && c1 <= 'z') c1 -= 32;
    if (c2 >= 'a' && c2 <= 'z') c2 -= 32;
    return c1 - c2;
}

char *basic_strdup(const char *src) {
    size_t len = strlen(src);
    char *dest = (char *)calloc(1, len + 1);
    if (dest) {
        memcpy(dest, src, len + 1);
    }
    return dest;
}

unsigned int hash_name(VariableContext *ctx, const char *name) {
    unsigned int hash = 2166136261u;
    while (*name) {
        char c = (ctx && ctx->case_sensitive) ? *name : (char)toupper((unsigned char)*name);
        hash ^= (unsigned char)c;
        hash *= 16777619u;
        name++;
    }
    return hash % HASH_BUCKETS;
}

void normalize_name(VariableContext *ctx, char *out, const char *in, size_t max_len) {
    size_t i = 0;
    while (in[i] && i < max_len - 1) {
        out[i] = (ctx && ctx->case_sensitive) ? in[i] : (char)toupper((unsigned char)in[i]);
        i++;
    }
    out[i] = '\0';
}

static void get_scoped_name(VariableContext *ctx, const char *name, char *out_buf, size_t out_max) {
    char norm[256];
    normalize_name(ctx, norm, name, sizeof(norm));

    bool is_shared = false;
    for (int i = 0; i < ctx->shared_count; ++i) {
        if (strcmp(ctx->shared_vars[i], norm) == 0) {
            is_shared = true;
            break;
        }
    }

    if (!is_shared && ctx->active_scope[0] != '\0') {
        snprintf(out_buf, out_max, "%s:%s", ctx->active_scope, norm);
    } else if (!is_shared && ctx->active_namespace[0] != '\0' && strchr(norm, '.') == NULL) {
        snprintf(out_buf, out_max, "%s.%s", ctx->active_namespace, norm);
    } else {
        strncpy(out_buf, norm, out_max - 1);
        out_buf[out_max - 1] = '\0';
    }
}

static char get_base_first_letter(const char *lookup_name) {
    const char *p = strchr(lookup_name, ':');
    if (p) {
        p++;
    } else {
        p = lookup_name;
    }
    const char *dot = strchr(p, '.');
    if (dot) {
        p = dot + 1;
    }
    while (*p && !isalpha((unsigned char)*p)) {
        p++;
    }
    if (*p) {
        return (char)toupper((unsigned char)*p);
    }
    return '\0';
}

//
// ---- Lookup & Assignment ----

BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing) {
    if (!ctx || !name || !*name) return NULL;

    bool is_special = is_magic_virtual_var(name);
    bool can_cache = (!is_special && ctx->active_scope[0] == '\0' && ctx->active_namespace[0] == '\0');

    if (can_cache) {
        for (int i = 0; i < VAR_CACHE_SIZE; ++i) {
            if (ctx->mru_cache[i].valid && basic_strcasecmp(ctx->mru_cache[i].name, name) == 0) {
                return ctx->mru_cache[i].val_ptr;
            }
        }
    }

    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));

    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];

    while (entry) {
        if (strcmp(entry->name, lookup_name) == 0) {
            BValue *res = &entry->value;
            if (basic_strcasecmp(name, "_CLIPBOARD$") == 0) {
                char *clip = platform_clipboard_get();
                if (clip) {
                    if (res->as.string) {
                        str_release(ctx->str, res->as.string);
                    }
                    res->as.string = str_create(ctx->str, clip, strlen(clip));
                    free(clip);
                }
            } else if (basic_strcasecmp(name, "HMOUSE") == 0 || basic_strcasecmp(name, "_HMOUSE") == 0) {
                int col = 1, row = 1;
                platform_mouse_get_position(&col, &row);
                res->type = VAL_NUMBER;
                res->as.number = col;
            } else if (basic_strcasecmp(name, "VMOUSE") == 0 || basic_strcasecmp(name, "_VMOUSE") == 0) {
                int col = 1, row = 1;
                platform_mouse_get_position(&col, &row);
                res->type = VAL_NUMBER;
                res->as.number = row;
            } else if (basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "_MOUSE") == 0 ||
                       basic_strcasecmp(name, "MOUSE$") == 0 || basic_strcasecmp(name, "_MOUSE$") == 0) {
                int col = 1, row = 1;
                platform_mouse_get_position(&col, &row);
                int hover_char = 32;
                extern VConContext *g_vcon_context;
                if (g_vcon_context) {
                    int active_idx = vcon_get_active_index(g_vcon_context);
                    hover_char = vcon_get_char_at(g_vcon_context, active_idx, row - 1, col - 1);
                }
                char buf[2] = {(char)hover_char, 0};
                if (res->type == VAL_STRING && res->as.string) {
                    str_release(ctx->str, res->as.string);
                }
                res->type = VAL_STRING;
                res->as.string = str_create(ctx->str, buf, 1);
            } else if (basic_strcasecmp(name, "TRIG") == 0 || basic_strcasecmp(name, "_TRIG") == 0) {
                int mask = 0;
                if (platform_mouse_get_button(0)) mask |= 1;
                if (platform_mouse_get_button(1)) mask |= 2;
                if (platform_mouse_get_button(2)) mask |= 4;
                res->type = VAL_NUMBER;
                res->as.number = mask;
            }
            if (can_cache) {
                uint8_t slot = ctx->mru_head;
                strncpy(ctx->mru_cache[slot].name, name, sizeof(ctx->mru_cache[slot].name) - 1);
                ctx->mru_cache[slot].name[sizeof(ctx->mru_cache[slot].name) - 1] = '\0';
                ctx->mru_cache[slot].val_ptr = res;
                ctx->mru_cache[slot].valid = true;
                ctx->mru_head = (slot + 1) & (VAR_CACHE_SIZE - 1);
            }
            return res;
        }
        entry = entry->next;
    }

    if (!create_if_missing) {
        return NULL;
    }

    if (ctx->is_explicit) {
        return NULL;
    }

    VarEntry *new_entry = (VarEntry *)calloc(1, sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = basic_strdup(lookup_name);
    if (!new_entry->name) {
        free(new_entry);
        return NULL;
    }

    size_t len = strlen(lookup_name);
    char last = lookup_name[len - 1];
    if (last == '$' || basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "_MOUSE") == 0) {
        new_entry->value.type = VAL_STRING;
        new_entry->value.as.string = NULL;
    } else if (last == '%') {
        new_entry->value.type = VAL_INTEGER;
        new_entry->value.as.number = 0.0;
    } else if (last == '&' || last == '!' || last == '#') {
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = 0.0;
    } else {
        char base_letter = get_base_first_letter(lookup_name);
        ValueType def_t = var_get_def_type(ctx, ctx->active_scope, base_letter);
        new_entry->value.type = def_t;
        if (def_t == VAL_STRING) {
            new_entry->value.as.string = NULL;
        } else if (def_t == VAL_COMPLEX) {
            new_entry->value.as.complex_val.real = 0.0;
            new_entry->value.as.complex_val.imag = 0.0;
        } else if (def_t == VAL_INTEGER) {
            new_entry->value.type = VAL_INTEGER;
            new_entry->value.as.number = 0.0;
        } else {
            new_entry->value.type = VAL_NUMBER;
            new_entry->value.as.number = 0.0;
        }
    }

    new_entry->next = ctx->buckets[bucket];
    ctx->buckets[bucket] = new_entry;

    if (basic_strcasecmp(name, "_CLIPBOARD$") == 0) {
        char *clip = platform_clipboard_get();
        if (clip) {
            new_entry->value.as.string = str_create(ctx->str, clip, strlen(clip));
            free(clip);
        }
    } else if (basic_strcasecmp(name, "HMOUSE") == 0 || basic_strcasecmp(name, "_HMOUSE") == 0) {
        int col = 1, row = 1;
        platform_mouse_get_position(&col, &row);
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = col;
    } else if (basic_strcasecmp(name, "VMOUSE") == 0 || basic_strcasecmp(name, "_VMOUSE") == 0) {
        int col = 1, row = 1;
        platform_mouse_get_position(&col, &row);
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = row;
    } else if (basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "_MOUSE") == 0 ||
               basic_strcasecmp(name, "MOUSE$") == 0 || basic_strcasecmp(name, "_MOUSE$") == 0) {
        int col = 1, row = 1;
        platform_mouse_get_position(&col, &row);
        int hover_char = 32;
        extern VConContext *g_vcon_context;
        if (g_vcon_context) {
            int active_idx = vcon_get_active_index(g_vcon_context);
            hover_char = vcon_get_char_at(g_vcon_context, active_idx, row - 1, col - 1);
        }
        char buf[2] = {(char)hover_char, 0};
        new_entry->value.type = VAL_STRING;
        new_entry->value.as.string = str_create(ctx->str, buf, 1);
    } else if (basic_strcasecmp(name, "TRIG") == 0 || basic_strcasecmp(name, "_TRIG") == 0) {
        int mask = 0;
        if (platform_mouse_get_button(0)) mask |= 1;
        if (platform_mouse_get_button(1)) mask |= 2;
        if (platform_mouse_get_button(2)) mask |= 4;
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = mask;
    }

    if (can_cache) {
        uint8_t slot = ctx->mru_head;
        strncpy(ctx->mru_cache[slot].name, name, sizeof(ctx->mru_cache[slot].name) - 1);
        ctx->mru_cache[slot].name[sizeof(ctx->mru_cache[slot].name) - 1] = '\0';
        ctx->mru_cache[slot].val_ptr = &new_entry->value;
        ctx->mru_cache[slot].valid = true;
        ctx->mru_head = (slot + 1) & (VAR_CACHE_SIZE - 1);
    }

    return &new_entry->value;
}

bool var_assign(VariableContext *ctx, const char *name, BValue val) {
    if (!ctx || !name || !*name) return false;

    BValue *var = var_lookup(ctx, name, true);
    if (!var) {
        return false;
    }

    if (basic_strcasecmp(name, "_CLIPBOARD$") == 0 && val.type == VAL_STRING) {
        const char *text = str_data(val.as.string);
        platform_clipboard_set(text ? text : "");
    } else if (basic_strcasecmp(name, "HMOUSE") == 0 || basic_strcasecmp(name, "_HMOUSE") == 0) {
        int col = 1, row = 1;
        platform_mouse_get_position(&col, &row);
        platform_mouse_set_position((int)val.as.number, row);
    } else if (basic_strcasecmp(name, "VMOUSE") == 0 || basic_strcasecmp(name, "_VMOUSE") == 0) {
        int col = 1, row = 1;
        platform_mouse_get_position(&col, &row);
        platform_mouse_set_position(col, (int)val.as.number);
    } else if (basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "_MOUSE") == 0 ||
               basic_strcasecmp(name, "MOUSE$") == 0 || basic_strcasecmp(name, "_MOUSE$") == 0) {
        if (val.type == VAL_STRING) {
            const char *str = str_data(val.as.string);
            if (str && str[0]) {
                platform_mouse_set_cursor(str[0], 7);
            }
        } else {
            platform_mouse_enable(val.as.number != 0.0);
        }
    }

    size_t name_len = strlen(name);
    char name_last = (name_len > 0) ? name[name_len - 1] : '\0';
    bool has_numeric_sigil = (name_last == '%' || name_last == '!' || name_last == '#' || name_last == '&');

    if (var->type == VAL_STRING || (val.type == VAL_STRING && !has_numeric_sigil && var_get_def_type(ctx, ctx->active_scope, get_base_first_letter(name)) != VAL_INTEGER)) {
        if (val.type == VAL_FIELD_STRING) {
            if (var->type == VAL_STRING && var->as.string) str_release(ctx->str, var->as.string);
            var->type = VAL_FIELD_STRING;
            var->as.field_str = val.as.field_str;
            return true;
        }
        if (val.type != VAL_STRING) {
            return false;
        }
        if (var->type == VAL_STRING && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->type = VAL_STRING;
        var->as.string = val.as.string;
        if (var->as.string) {
            str_add_ref(var->as.string);
        }
        return true;
    } else if (var->type == VAL_FIELD_STRING) {
        if (val.type == VAL_FIELD_STRING) {
            var->as.field_str = val.as.field_str;
            return true;
        }
        return false;
    } else if (var->type == VAL_MAP) {
        if (val.type != VAL_MAP) {
            return false;
        }
        if (var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->as.map = val.as.map;
        if (var->as.map) {
            map_add_ref(var->as.map);
        }
    } else if (val.type == VAL_MAP) {
        if ((var->type == VAL_STRING && name_last == '$') || (var->type == VAL_INTEGER && name_last == '%')) {
            return false;
        }
        var->type = VAL_MAP;
        var->as.map = val.as.map;
        if (var->as.map) {
            map_add_ref(var->as.map);
        }
    } else if (val.type == VAL_ARRAY_REF) {
        if (var->type == VAL_STRING || var->type == VAL_INTEGER) {
            return false;
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->type = VAL_ARRAY_REF;
        var->as.string = val.as.string;
        if (var->as.string) {
            str_add_ref(var->as.string);
        }
    } else if (var->type == VAL_INTEGER) {
        if (val.type == VAL_STRING) {
            return false;
        }
        var->as.number = (double)((int32_t)val.as.number);
    } else if (val.type == VAL_COMPLEX || var->type == VAL_COMPLEX) {
        if (val.type == VAL_STRING) {
            return false;
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->type = VAL_COMPLEX;
        if (val.type == VAL_COMPLEX) {
            var->as.complex_val = val.as.complex_val;
        } else {
            var->as.complex_val.real = val.as.number;
            var->as.complex_val.imag = 0.0;
        }
    } else {
        if (val.type == VAL_STRING) {
            return false;
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->type = VAL_NUMBER;
        var->as.number = val.as.number;
    }

    return true;
}

BValue *var_declare(VariableContext *ctx, const char *name) {
    if (!ctx || !name || !*name) return NULL;

    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));

    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];

    while (entry) {
        if (strcmp(entry->name, lookup_name) == 0) {
            return &entry->value;
        }
        entry = entry->next;
    }

    VarEntry *new_entry = (VarEntry *)calloc(1, sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = basic_strdup(lookup_name);
    if (!new_entry->name) {
        free(new_entry);
        return NULL;
    }

    size_t len = strlen(lookup_name);
    char last = lookup_name[len - 1];

    if (last == '$') {
        new_entry->value.type = VAL_STRING;
        new_entry->value.as.string = NULL;
    } else if (last == '%') {
        new_entry->value.type = VAL_INTEGER;
        new_entry->value.as.number = 0.0;
    } else if (last == '&' || last == '!' || last == '#') {
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = 0.0;
    } else {
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = 0.0;
    }

    new_entry->next = ctx->buckets[bucket];
    ctx->buckets[bucket] = new_entry;

    return &new_entry->value;
}
