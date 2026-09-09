// FILENAME: var_lookup.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for var_lookup within BASIC++.

#include "runtime/variables_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/memory/alloc.h"
#include "platform/platform.h"

//
// ---- String & Hash Helpers ----
//

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
    size_t len = runtime_strlen(src);
    char *dest = (char *)runtime_calloc(1, len + 1);
    if (dest) {
        runtime_memcpy(dest, src, len + 1);
    }
    return dest;
}

unsigned int hash_name(VariableContext *ctx, const char *name) {
    unsigned int hash = 2166136261u;
    while (*name) {
        char c = (ctx && ctx->case_sensitive) ? *name : (char)runtime_toupper((unsigned char)*name);
        hash ^= (unsigned char)c;
        hash *= 16777619u;
        name++;
    }
    return hash % HASH_BUCKETS;
}

void normalize_name(VariableContext *ctx, char *out, const char *in, size_t max_len) {
    size_t i = 0;
    while (in[i] && i < max_len - 1) {
        out[i] = (ctx && ctx->case_sensitive) ? in[i] : (char)runtime_toupper((unsigned char)in[i]);
        i++;
    }
    out[i] = '\0';
}

static void get_scoped_name(VariableContext *ctx, const char *name, char *out_buf, size_t out_max) {
    char norm[256];
    normalize_name(ctx, norm, name, sizeof(norm));

    bool is_shared = false;
    for (int i = 0; i < ctx->shared_count; ++i) {
        if (runtime_strcmp(ctx->shared_vars[i], norm) == 0) {
            is_shared = true;
            break;
        }
    }

    if (!is_shared && ctx->active_scope[0] != '\0') {
        runtime_snprintf(out_buf, out_max, "%s:%s", ctx->active_scope, norm);
    } else if (!is_shared && ctx->active_namespace[0] != '\0' && runtime_strchr(norm, '.') == NULL) {
        runtime_snprintf(out_buf, out_max, "%s.%s", ctx->active_namespace, norm);
    } else {
        runtime_strncpy(out_buf, norm, out_max - 1);
        out_buf[out_max - 1] = '\0';
    }
}

static char get_base_first_letter(const char *lookup_name) {
    const char *p = runtime_strchr(lookup_name, ':');
    if (p) {
        p++;
    } else {
        p = lookup_name;
    }
    const char *dot = runtime_strchr(p, '.');
    if (dot) {
        p = dot + 1;
    }
    while (*p && !runtime_isalpha((unsigned char)*p)) {
        p++;
    }
    if (*p) {
        return (char)runtime_toupper((unsigned char)*p);
    }
    return '\0';
}

//
// ---- Magic & Dynamic Variable Helpers ----
//

static void update_magic_var_value(VariableContext *ctx, const char *name, BValue *res) {
    if (!ctx || !name || !res) return;

    if (basic_strcasecmp(name, "_CLIPBOARD$") == 0) {
        char *clip = platform_clipboard_get();
        if (clip) {
            if (res->type == VAL_STRING && res->as.string) {
                str_release(ctx->str, res->as.string);
            }
            res->type = VAL_STRING;
            res->as.string = str_create(ctx->str, clip, runtime_strlen(clip));
            platform_clipboard_free(clip);
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
    } else if (basic_strcasecmp(name, "ST") == 0 || basic_strcasecmp(name, "_ST") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 0.0;
    } else if (basic_strcasecmp(name, "DSTATS") == 0 || basic_strcasecmp(name, "DSTAT") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 0.0;
    } else if (basic_strcasecmp(name, "SOFTEV") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 1010.0;
    } else if (basic_strcasecmp(name, "PWRED") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 165.0;
    } else if (basic_strcasecmp(name, "RESET_VECTOR") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 64098.0;
    } else if (basic_strcasecmp(name, "_BATTERY%") == 0 || basic_strcasecmp(name, "_BATTERY") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = (double)platform_get_battery_level();
    } else if (basic_strcasecmp(name, "_TEMPERATURE") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = platform_get_temperature();
    } else if (basic_strcasecmp(name, "_CPU_LOAD%") == 0 || basic_strcasecmp(name, "_CPU_LOAD") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = (double)platform_get_cpu_load();
    } else if (basic_strcasecmp(name, "_WIFI_RSSI%") == 0 || basic_strcasecmp(name, "_WIFI_RSSI") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = (double)platform_get_wifi_rssi();
    } else if (basic_strcasecmp(name, "_FREE_STACK") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = 1048576.0;
    } else if (basic_strcasecmp(name, "_TASK_ID") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = 0.0;
    } else if (basic_strcasecmp(name, "_TASK_COUNT") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = 1.0;
    } else if (basic_strcasecmp(name, "_ALARM_PENDING") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = 0.0;
    } else if (basic_strcasecmp(name, "_TRAP_FLAGS") == 0) {
        res->type = VAL_INTEGER;
        res->as.number = 0.0;
    } else if (basic_strcasecmp(name, "MEM") == 0) {
        res->type = VAL_NUMBER;
        res->as.number = (double)(ctx->mem ? mem_get_total_ram(ctx->mem) : 671088640L);
    }
}

DynamicVarEntry *var_find_dynamic(VariableContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;
    DynamicVarEntry *cur = ctx->dynamic_vars;
    while (cur) {
        if (basic_strcasecmp(cur->name, name) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

bool var_register_dynamic(VariableContext *ctx, const char *name, BppVarGetter getter, BppVarSetter setter, void *user_data) {
    if (!ctx || !name || !*name) return false;
    DynamicVarEntry *entry = var_find_dynamic(ctx, name);
    if (!entry) {
        entry = (DynamicVarEntry *)runtime_calloc(1, sizeof(DynamicVarEntry));
        if (!entry) return false;
        runtime_strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->next = ctx->dynamic_vars;
        ctx->dynamic_vars = entry;
    }
    entry->getter = getter;
    entry->setter = setter;
    entry->user_data = user_data;
    return true;
}

bool var_register_basic_dynamic(VariableContext *ctx, const char *name, const char *read_fn, const char *write_fn) {
    if (!ctx || !name || !*name) return false;
    DynamicVarEntry *entry = var_find_dynamic(ctx, name);
    if (!entry) {
        entry = (DynamicVarEntry *)runtime_calloc(1, sizeof(DynamicVarEntry));
        if (!entry) return false;
        runtime_strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->next = ctx->dynamic_vars;
        ctx->dynamic_vars = entry;
    }
    if (read_fn) runtime_strncpy(entry->read_fn, read_fn, sizeof(entry->read_fn) - 1);
    if (write_fn) runtime_strncpy(entry->write_fn, write_fn, sizeof(entry->write_fn) - 1);
    return true;
}

//
// ---- Lookup & Assignment ----
//

BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing) {
    if (!ctx || !name || !*name) return NULL;

    bool is_special = is_magic_virtual_var(name);
    bool can_cache = (!is_special && ctx->active_scope[0] == '\0' && ctx->active_namespace[0] == '\0');

    // 1. Ultra-fast path: Single-letter numeric variable (e.g. A..Z, i, j, k, x, y, r, c)
    if (can_cache && ctx->shared_count == 0) {
        char c0 = name[0];
        char c1 = name[1];
        if ((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z')) {
            if (c1 == '\0' || ((c1 == '%' || c1 == '!' || c1 == '#' || c1 == '&') && name[2] == '\0')) {
                int letter_idx = (c0 >= 'a') ? (c0 - 'a') : (c0 - 'A');
                if (ctx->global_def_types[letter_idx] != VAL_STRING) {
                    if (ctx->fast_scalars_valid[letter_idx]) {
                        return &ctx->fast_scalars[letter_idx];
                    }
                    if (create_if_missing) {
                        ValueType dt = ctx->global_def_types[letter_idx];
                        bool is_int = (c1 == '%' || dt == VAL_INTEGER);
                        ctx->fast_scalars[letter_idx].type = is_int ? VAL_INTEGER : VAL_NUMBER;
                        ctx->fast_scalars[letter_idx].as.number = 0.0;
                        ctx->fast_scalars_valid[letter_idx] = true;
                        return &ctx->fast_scalars[letter_idx];
                    }
                    return NULL;
                }
            }
        }
    }

    // 2. Direct-mapped 256-slot hash cache
    uint32_t name_h = 2166136261u;
    if (can_cache) {
        for (const char *p = name; *p; p++) {
            char c = (ctx->case_sensitive) ? *p : (char)runtime_toupper((unsigned char)*p);
            name_h ^= (unsigned char)c;
            name_h *= 16777619u;
        }
        uint32_t slot = name_h & (DIRECT_VAR_CACHE_SIZE - 1);
        if (ctx->direct_cache[slot].valid && ctx->direct_cache[slot].hash == name_h) {
            if (basic_strcasecmp(ctx->direct_cache[slot].name, name) == 0) {
                return ctx->direct_cache[slot].val_ptr;
            }
        }
    }

    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));

    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];

    while (entry) {
        if (runtime_strcmp(entry->name, lookup_name) == 0) {
            BValue *res = &entry->value;
            update_magic_var_value(ctx, name, res);
            DynamicVarEntry *dvar = var_find_dynamic(ctx, name);
            if (dvar && dvar->getter) {
                *res = dvar->getter(ctx, name, dvar->user_data);
            }
            if (can_cache) {
                uint32_t slot = name_h & (DIRECT_VAR_CACHE_SIZE - 1);
                runtime_strncpy(ctx->direct_cache[slot].name, name, sizeof(ctx->direct_cache[slot].name) - 1);
                ctx->direct_cache[slot].name[sizeof(ctx->direct_cache[slot].name) - 1] = '\0';
                ctx->direct_cache[slot].hash = name_h;
                ctx->direct_cache[slot].val_ptr = res;
                ctx->direct_cache[slot].valid = true;
            }
            return res;
        }
        entry = entry->next;
    }

    if (!create_if_missing && !is_special && !var_find_dynamic(ctx, name)) {
        return NULL;
    }

    if (ctx->is_explicit && !is_special && !var_find_dynamic(ctx, name)) {
        return NULL;
    }

    VarEntry *new_entry = (VarEntry *)runtime_calloc(1, sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = basic_strdup(lookup_name);
    if (!new_entry->name) {
        runtime_free(new_entry);
        return NULL;
    }

    size_t len = runtime_strlen(lookup_name);
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

    update_magic_var_value(ctx, name, &new_entry->value);
    DynamicVarEntry *dvar = var_find_dynamic(ctx, name);
    if (dvar && dvar->getter) {
        new_entry->value = dvar->getter(ctx, name, dvar->user_data);
    }

    if (can_cache) {
        uint32_t slot = name_h & (DIRECT_VAR_CACHE_SIZE - 1);
        runtime_strncpy(ctx->direct_cache[slot].name, name, sizeof(ctx->direct_cache[slot].name) - 1);
        ctx->direct_cache[slot].name[sizeof(ctx->direct_cache[slot].name) - 1] = '\0';
        ctx->direct_cache[slot].hash = name_h;
        ctx->direct_cache[slot].val_ptr = &new_entry->value;
        ctx->direct_cache[slot].valid = true;
    }

    return &new_entry->value;
}

bool var_assign(VariableContext *ctx, const char *name, BValue val) {
    if (!ctx || !name || !*name) return false;

    // Ultra-Fast Path: Single-letter global scalar assignment (e.g. X, Y, I, J, K, A..Z)
    if (ctx->active_scope[0] == '\0' && ctx->active_namespace[0] == '\0' && ctx->shared_count == 0) {
        char c0 = name[0];
        char c1 = name[1];
        if ((c0 >= 'A' && c0 <= 'Z') || (c0 >= 'a' && c0 <= 'z')) {
            if (c1 == '\0' || ((c1 == '%' || c1 == '!' || c1 == '#' || c1 == '&') && name[2] == '\0')) {
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    int idx = (c0 >= 'a') ? (c0 - 'a') : (c0 - 'A');
                    ValueType dt = ctx->global_def_types[idx];
                    if (dt != VAL_STRING) {
                        bool is_int = (c1 == '%' || dt == VAL_INTEGER);
                        ctx->fast_scalars[idx].type = is_int ? VAL_INTEGER : VAL_NUMBER;
                        ctx->fast_scalars[idx].as.number = is_int ? (double)((int32_t)val.as.number) : val.as.number;
                        ctx->fast_scalars_valid[idx] = true;
                        return true;
                    }
                }
            }
        }
    }

    DynamicVarEntry *dvar = var_find_dynamic(ctx, name);
    if (dvar && dvar->setter) {
        return dvar->setter(ctx, name, val, dvar->user_data);
    }

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

    size_t name_len = runtime_strlen(name);
    char name_last = (name_len > 0) ? name[name_len - 1] : '\0';
    bool has_numeric_sigil = (name_last == '%' || name_last == '!' || name_last == '#' || name_last == '&');
    bool has_string_sigil = (name_last == '$');

    if (has_string_sigil || var->type == VAL_STRING || (val.type == VAL_STRING && !has_numeric_sigil && var_get_def_type(ctx, ctx->active_scope, get_base_first_letter(name)) != VAL_INTEGER)) {
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
        size_t max_len = var_get_max_len(ctx, name);
        if (max_len > 0 && val.as.string != NULL && str_len(val.as.string) > max_len) {
            var->as.string = str_create(ctx->str, str_data(val.as.string), max_len);
        } else {
            var->as.string = val.as.string;
            if (var->as.string) {
                str_add_ref(var->as.string);
            }
        }
        return true;
    } else if (var->type == VAL_FIELD_STRING) {
        if (val.type == VAL_FIELD_STRING) {
            var->as.field_str = val.as.field_str;
            return true;
        } else if (val.type == VAL_STRING) {
            // Authentic vintage GW-BASIC field detachment: unbind from sector buffer to dynamic string
            var->type = VAL_STRING;
            var->as.string = val.as.string;
            if (var->as.string) {
                str_add_ref(var->as.string);
            }
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
    } else if (var->type == VAL_SET) {
        if (val.type != VAL_SET) {
            return false;
        }
        if (var->as.set) {
            set_release(ctx->str, var->as.set);
        }
        var->as.set = val.as.set;
        if (var->as.set) {
            set_add_ref(var->as.set);
        }
    } else if (val.type == VAL_SET) {
        if ((var->type == VAL_STRING && name_last == '$') || (var->type == VAL_INTEGER && name_last == '%')) {
            return false;
        }
        if (var->type == VAL_STRING && var->as.string) str_release(ctx->str, var->as.string);
        else if (var->type == VAL_MAP && var->as.map) map_release(ctx->str, var->as.map);
        else if (var->type == VAL_SET && var->as.set) set_release(ctx->str, var->as.set);
        else if (var->type == VAL_GROUP && var->as.group) group_release(ctx->str, var->as.group);
        var->type = VAL_SET;
        var->as.set = val.as.set;
        if (var->as.set) {
            set_add_ref(var->as.set);
        }
    } else if (var->type == VAL_GROUP) {
        if (val.type != VAL_GROUP) {
            return false;
        }
        if (var->as.group) {
            group_release(ctx->str, var->as.group);
        }
        var->as.group = val.as.group;
        if (var->as.group) {
            group_add_ref(var->as.group);
        }
    } else if (val.type == VAL_GROUP) {
        if ((var->type == VAL_STRING && name_last == '$') || (var->type == VAL_INTEGER && name_last == '%')) {
            return false;
        }
        if (var->type == VAL_STRING && var->as.string) str_release(ctx->str, var->as.string);
        else if (var->type == VAL_MAP && var->as.map) map_release(ctx->str, var->as.map);
        else if (var->type == VAL_SET && var->as.set) set_release(ctx->str, var->as.set);
        else if (var->type == VAL_GROUP && var->as.group) group_release(ctx->str, var->as.group);
        var->type = VAL_GROUP;
        var->as.group = val.as.group;
        if (var->as.group) {
            group_add_ref(var->as.group);
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
    } else if (var->type == VAL_INTEGER || (!has_numeric_sigil && var_get_def_type(ctx, ctx->active_scope, get_base_first_letter(name)) == VAL_INTEGER)) {
        if (val.type == VAL_STRING) {
            return false;
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            map_release(ctx->str, var->as.map);
        }
        var->type = VAL_INTEGER;
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
        if (runtime_strcmp(entry->name, lookup_name) == 0) {
            return &entry->value;
        }
        entry = entry->next;
    }

    VarEntry *new_entry = (VarEntry *)runtime_calloc(1, sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = basic_strdup(lookup_name);
    if (!new_entry->name) {
        runtime_free(new_entry);
        return NULL;
    }

    size_t len = runtime_strlen(lookup_name);
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

void var_set_max_len(VariableContext *ctx, const char *name, size_t max_len) {
    if (!ctx || !name || !*name) return;
    var_declare(ctx, name);
    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));
    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, lookup_name) == 0) {
            entry->max_len = max_len;
            if (max_len > 0 && entry->value.type == VAL_STRING && entry->value.as.string) {
                size_t slen = str_len(entry->value.as.string);
                if (slen > max_len) {
                    BppStringRef truncated = str_create(ctx->str, str_data(entry->value.as.string), max_len);
                    str_release(ctx->str, entry->value.as.string);
                    entry->value.as.string = truncated;
                }
            }
            return;
        }
        entry = entry->next;
    }
}

size_t var_get_max_len(VariableContext *ctx, const char *name) {
    if (!ctx || !name || !*name) return 0;
    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));
    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];
    while (entry) {
        if (runtime_strcmp(entry->name, lookup_name) == 0) {
            return entry->max_len;
        }
        entry = entry->next;
    }
    return 0;
}

