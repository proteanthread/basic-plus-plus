/**
 * @file variables.c
 * @brief Variable storage and lookup table implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements case-insensitive variable storage, retrieval, and assignments.
 *   Handles implicit creation of variables, type suffixes (%, &, !, #, $), and OPTION EXPLICIT checks.
 * - Why it exists: Centralizes interpreter state variables, ensuring type constraints are validated
 *   upon assignment and preventing access to uninitialized memory.
 * - Why it works this way: It uses a chained hash map with 128 buckets for O(1) average lookup times.
 *   Names are normalized to upper case before hashing, meaning "var$" and "VAR$" resolve to the same entry,
 *   while "A%" and "A$" remain distinct entries due to suffix differences.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Suffix mapping rules, hash table bucket size (128 is default).
 * - What cannot be changed: Case-insensitivity rules and strict type suffix mappings.
 * - What to expect: Assigning a string to an integer variable (%) will trigger a type mismatch error.
 * - What to do if something breaks: If a lookup returns NULL or a variable's value changes unexpectedly,
 *   trace the upper-case normalization function and hash calculations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Character encoding is standard ASCII. Suffixes are limited to standard BASIC symbols.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Dynamic array scopes, nested structures, or local variable frame links.
 * - How to write external extensions: Plugins access variable values via the var_lookup and var_assign API signatures.
 */

#include "bpp_variables.h"
#include "bpp_map.h"
#include "bpp_vdev.h"
#include "bpp_platform.h"
#include "bpp_logger.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define HASH_BUCKETS 128

typedef struct VarEntry {
    char            *name;   /* Upper-case normalized variable name */
    BValue           value;  /* Stored value structure */
    struct VarEntry *next;   /* Next entry in bucket chain */
} VarEntry;

typedef struct ScopeDefMapping {
    char                    scope_name[128];
    ValueType               def_types[26];
    struct ScopeDefMapping *next;
} ScopeDefMapping;

struct VariableContext {
    MemoryContext   *mem;
    StringContext   *str;
    VarEntry        *buckets[HASH_BUCKETS];
    bool             is_explicit;
    char             active_scope[256];
    char             active_namespace[64];
    char             shared_vars[64][64];
    int              shared_count;
    ValueType        global_def_types[26];
    ScopeDefMapping *scope_defs;
    bool             case_sensitive;
};

/* Simple case-insensitive or case-sensitive hash function */
static unsigned int hash_name(VariableContext *ctx, const char *name) {
    unsigned int hash = 5381;
    bool cs = ctx ? ctx->case_sensitive : false;
    while (*name) {
        char c = *name;
        if (!cs) {
            c = (char)toupper((unsigned char)c);
        }
        hash = ((hash << 5) + hash) + c;
        name++;
    }
    return hash % HASH_BUCKETS;
}

/* Helper to normalize name to upper case or preserve case */
static void normalize_name(VariableContext *ctx, char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    bool cs = ctx ? ctx->case_sensitive : false;
    while (src[i] && i < max_len - 1) {
        dest[i] = cs ? src[i] : (char)toupper((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

/* Portable C17 strdup replacement to avoid POSIX warnings and dependencies */
static char *bpp_strdup(const char *src) {
    size_t len = strlen(src);
    char *dest = (char *)malloc(len + 1);
    if (dest) {
        memcpy(dest, src, len + 1);
    }
    return dest;
}

static int bpp_strcasecmp(const char *s1, const char *s2) {
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

VariableContext *var_init(MemoryContext *mem, StringContext *str) {
    if (!mem || !str) return NULL;
    VariableContext *ctx = (VariableContext *)malloc(sizeof(VariableContext));
    if (!ctx) return NULL;
    ctx->mem = mem;
    ctx->str = str;
    ctx->is_explicit = false;
    ctx->active_scope[0] = '\0';
    ctx->active_namespace[0] = '\0';
    ctx->shared_count = 0;
    memset(ctx->buckets, 0, sizeof(ctx->buckets));
    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
    ctx->scope_defs = NULL;
    ctx->case_sensitive = false;
    return ctx;
}

void var_shutdown(VariableContext *ctx) {
    if (!ctx) return;
    var_clear_all(ctx);
    free(ctx);
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

BValue *var_lookup(VariableContext *ctx, const char *name, bool create_if_missing) {
    if (!ctx || !name || !*name) return NULL;

    char lookup_name[512];
    get_scoped_name(ctx, name, lookup_name, sizeof(lookup_name));

    unsigned int bucket = hash_name(ctx, lookup_name);
    VarEntry *entry = ctx->buckets[bucket];

    while (entry) {
        if (strcmp(entry->name, lookup_name) == 0) {
            BValue *res = &entry->value;
            if (bpp_strcasecmp(name, "_CLIPBOARD$") == 0) {
                char *clip = platform_clipboard_get();
                if (clip) {
                    if (res->as.string) {
                        str_release(ctx->str, res->as.string);
                    }
                    res->as.string = str_create(ctx->str, clip, strlen(clip));
                    free(clip);
                }
            }
            return res;
        }
        entry = entry->next;
    }

    if (!create_if_missing) {
        return NULL;
    }

    if (ctx->is_explicit) {
        /* OPTION EXPLICIT is enabled: cannot implicitly declare variables */
        return NULL;
    }

    /* Implicitly declare the variable */
    VarEntry *new_entry = (VarEntry *)malloc(sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = bpp_strdup(lookup_name);
    if (!new_entry->name) {
        free(new_entry);
        return NULL;
    }

    /* Determine type based on suffix or overrides */
    size_t len = strlen(lookup_name);
    char last = lookup_name[len - 1];

    if (last == '$') {
        new_entry->value.type = VAL_STRING;
        new_entry->value.as.string = NULL; /* Empty string */
    } else if (last == '%') {
        new_entry->value.type = VAL_INTEGER;
        new_entry->value.as.number = 0.0;
    } else if (last == '&' || last == '!' || last == '#') {
        new_entry->value.type = VAL_NUMBER;
        new_entry->value.as.number = 0.0;
    } else {
        char base_letter = get_base_first_letter(lookup_name);
        new_entry->value.type = var_get_def_type(ctx, ctx->active_scope, base_letter);
        if (new_entry->value.type == VAL_STRING) {
            new_entry->value.as.string = NULL;
        } else {
            new_entry->value.as.number = 0.0;
        }
    }

    new_entry->next = ctx->buckets[bucket];
    ctx->buckets[bucket] = new_entry;

    if (bpp_strcasecmp(name, "_CLIPBOARD$") == 0) {
        char *clip = platform_clipboard_get();
        if (clip) {
            new_entry->value.as.string = str_create(ctx->str, clip, strlen(clip));
            free(clip);
        }
    }

    return &new_entry->value;
}

bool var_assign(VariableContext *ctx, const char *name, BValue val) {
    if (!ctx || !name || !*name) return false;

    BValue *var = var_lookup(ctx, name, true);
    if (!var) {
        return false; /* Explicit declaration violation or OOM */
    }

    if (bpp_strcasecmp(name, "_CLIPBOARD$") == 0 && val.type == VAL_STRING) {
        const char *text = str_data(val.as.string);
        platform_clipboard_set(text ? text : "");
    }

    /* Check type compatibility */
    if (var->type == VAL_STRING) {
        if (val.type == VAL_FIELD_STRING) {
            if (var->as.string) str_release(ctx->str, var->as.string);
            var->type = VAL_FIELD_STRING;
            var->as.field_str = val.as.field_str;
            return true;
        }
        if (val.type != VAL_STRING) {
            return false; /* Type mismatch */
        }
        /* Release old string, reference new one */
        if (var->as.string) {
            str_release(ctx->str, var->as.string);
        }
        var->as.string = val.as.string;
        if (var->as.string) {
            str_add_ref(var->as.string);
        }
    } else if (var->type == VAL_FIELD_STRING) {
        if (val.type == VAL_FIELD_STRING) {
            var->as.field_str = val.as.field_str;
            return true;
        }
        return false; /* Cannot directly assign non-field string without LSET */
    } else if (var->type == VAL_MAP) {
        if (val.type != VAL_MAP) {
            return false; /* Type mismatch */
        }
        if (var->as.map) {
            bpp_map_release(ctx->str, var->as.map);
        }
        var->as.map = val.as.map;
        if (var->as.map) {
            bpp_map_add_ref(var->as.map);
        }
    } else if (val.type == VAL_MAP) {
        if (var->type == VAL_STRING || var->type == VAL_INTEGER) {
            return false; /* Type mismatch */
        }
        var->type = VAL_MAP;
        var->as.map = val.as.map;
        if (var->as.map) {
            bpp_map_add_ref(var->as.map);
        }
    } else if (val.type == VAL_ARRAY_REF) {
        if (var->type == VAL_STRING || var->type == VAL_INTEGER) {
            return false; /* Type mismatch */
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            bpp_map_release(ctx->str, var->as.map);
        }
        var->type = VAL_ARRAY_REF;
        var->as.string = val.as.string;
        if (var->as.string) {
            str_add_ref(var->as.string);
        }
    } else if (var->type == VAL_INTEGER) {
        if (val.type == VAL_STRING) {
            return false; /* Type mismatch */
        }
        /* Convert float/double to integer value */
        var->as.number = (double)((int32_t)val.as.number);
    } else {
        if (val.type == VAL_STRING) {
            return false; /* Type mismatch */
        }
        if (var->type == VAL_ARRAY_REF && var->as.string) {
            str_release(ctx->str, var->as.string);
        } else if (var->type == VAL_MAP && var->as.map) {
            bpp_map_release(ctx->str, var->as.map);
        }
        var->type = VAL_NUMBER;
        var->as.number = val.as.number;
    }

    return true;
}

static void clear_scope_defs(VariableContext *ctx) {
    ScopeDefMapping *curr = ctx->scope_defs;
    while (curr) {
        ScopeDefMapping *next = curr->next;
        free(curr);
        curr = next;
    }
    ctx->scope_defs = NULL;
}

void var_clear_all(VariableContext *ctx) {
    if (!ctx) return;

    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            VarEntry *next = entry->next;
            free(entry->name);
            if ((entry->value.type == VAL_STRING || entry->value.type == VAL_ARRAY_REF) && entry->value.as.string) {
                str_release(ctx->str, entry->value.as.string);
            } else if (entry->value.type == VAL_MAP && entry->value.as.map) {
                bpp_map_release(ctx->str, entry->value.as.map);
            }
            free(entry);
            entry = next;
        }
        ctx->buckets[i] = NULL;
    }
    ctx->is_explicit = false;
    ctx->shared_count = 0;
    clear_scope_defs(ctx);
    for (int i = 0; i < 26; ++i) {
        ctx->global_def_types[i] = VAL_NUMBER;
    }
}

void var_set_explicit(VariableContext *ctx, bool enable) {
    if (ctx) {
        ctx->is_explicit = enable;
    }
}

void var_set_case_sensitive(VariableContext *ctx, bool enable) {
    if (ctx) {
        ctx->case_sensitive = enable;
    }
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

    VarEntry *new_entry = (VarEntry *)malloc(sizeof(VarEntry));
    if (!new_entry) return NULL;

    new_entry->name = bpp_strdup(lookup_name);
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

bool var_is_explicit(VariableContext *ctx) {
    return ctx ? ctx->is_explicit : false;
}

void var_clear_scope(VariableContext *ctx, const char *prefix) {
    if (!ctx || !prefix || prefix[0] == '\0') return;

    char norm_prefix[256];
    size_t plen = 0;
    while (prefix[plen] && plen < sizeof(norm_prefix) - 1) {
        norm_prefix[plen] = (char)toupper((unsigned char)prefix[plen]);
        plen++;
    }
    norm_prefix[plen] = '\0';

    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *prev = NULL;
        VarEntry *curr = ctx->buckets[i];
        while (curr) {
            if (strncmp(curr->name, norm_prefix, plen) == 0) {
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
                    bpp_map_release(ctx->str, temp->value.as.map);
                }
                free(temp->name);
                free(temp);
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }
}
void var_set_scope(VariableContext *ctx, const char *scope) {
    if (!ctx) return;
    if (scope) {
        strncpy(ctx->active_scope, scope, sizeof(ctx->active_scope) - 1);
        ctx->active_scope[sizeof(ctx->active_scope) - 1] = '\0';
    } else {
        ctx->active_scope[0] = '\0';
    }
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
        strncpy(ctx->active_namespace, ns, sizeof(ctx->active_namespace) - 1);
        ctx->active_namespace[sizeof(ctx->active_namespace) - 1] = '\0';
    } else {
        ctx->active_namespace[0] = '\0';
    }
}

void var_set_def_type(VariableContext *ctx, const char *scope, char start_letter, char end_letter, ValueType type) {
    if (!ctx) return;
    char start = (char)toupper((unsigned char)start_letter);
    char end = (char)toupper((unsigned char)end_letter);
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
            if (strcmp(curr->scope_name, scope) == 0) {
                break;
            }
            curr = curr->next;
        }
        if (!curr) {
            curr = (ScopeDefMapping *)malloc(sizeof(ScopeDefMapping));
            if (!curr) return;
            strncpy(curr->scope_name, scope, sizeof(curr->scope_name) - 1);
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
            ctx->global_def_types[c - 'A'] = type;
        }
    }
}

ValueType var_get_def_type(VariableContext *ctx, const char *scope, char letter) {
    if (!ctx) return VAL_NUMBER;
    char l = (char)toupper((unsigned char)letter);
    if (l < 'A' || l > 'Z') return VAL_NUMBER;
    int idx = l - 'A';

    if (scope && scope[0] != '\0') {
        ScopeDefMapping *curr = ctx->scope_defs;
        while (curr) {
            if (strcmp(curr->scope_name, scope) == 0) {
                return curr->def_types[idx];
            }
            curr = curr->next;
        }
    }
    return ctx->global_def_types[idx];
}

void var_print_all(VariableContext *ctx, void *vdev_ptr) {
    if (!ctx) return;
    VDevContext *vdev = (VDevContext *)vdev_ptr;

    /* Print header */
    if (vdev) {
        vdev_printf(vdev, "--- Active Variables ---\n");
    }
    bpp_log_info("--- Active Variables ---");

    int count = 0;
    for (int i = 0; i < HASH_BUCKETS; ++i) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            char val_buf[256] = {0};
            if (entry->value.type == VAL_NUMBER) {
                snprintf(val_buf, sizeof(val_buf), "%g", entry->value.as.number);
            } else if (entry->value.type == VAL_STRING) {
                if (entry->value.as.string) {
                    snprintf(val_buf, sizeof(val_buf), "\"%s\"", str_data(entry->value.as.string));
                } else {
                    snprintf(val_buf, sizeof(val_buf), "\"\"");
                }
            } else if (entry->value.type == VAL_INTEGER) {
                snprintf(val_buf, sizeof(val_buf), "%d", (int)entry->value.as.number);
            } else {
                snprintf(val_buf, sizeof(val_buf), "<complex or unhandled type>");
            }

            if (vdev) {
                vdev_printf(vdev, "  %s = %s\n", entry->name, val_buf);
            }
            bpp_log_info("  %s = %s", entry->name, val_buf);
            count++;
            entry = entry->next;
        }
    }

    if (vdev) {
        vdev_printf(vdev, "Total variables: %d\n", count);
    }
    bpp_log_info("Total variables: %d", count);
}

bool var_serialize(VariableContext *ctx, void *fp) {
    if (!ctx || !fp) return false;
    FILE *f = (FILE *)fp;

    uint32_t count = 0;
    for (int i = 0; i < HASH_BUCKETS; i++) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            count++;
            entry = entry->next;
        }
    }

    fwrite(&count, sizeof(count), 1, f);

    for (int i = 0; i < HASH_BUCKETS; i++) {
        VarEntry *entry = ctx->buckets[i];
        while (entry) {
            uint32_t name_len = (uint32_t)strlen(entry->name);
            fwrite(&name_len, sizeof(name_len), 1, f);
            fwrite(entry->name, 1, name_len, f);

            uint32_t type = (uint32_t)entry->value.type;
            fwrite(&type, sizeof(type), 1, f);

            if (entry->value.type == VAL_NUMBER || entry->value.type == VAL_INTEGER) {
                fwrite(&entry->value.as.number, sizeof(double), 1, f);
            } else if (entry->value.type == VAL_STRING) {
                const char *s = str_data(entry->value.as.string);
                uint32_t s_len = s ? (uint32_t)strlen(s) : 0;
                fwrite(&s_len, sizeof(s_len), 1, f);
                if (s_len > 0) {
                    fwrite(s, 1, s_len, f);
                }
            } else if (entry->value.type == VAL_ARRAY_REF) {
                uint32_t a_len = entry->value.as.array_name ? (uint32_t)strlen(entry->value.as.array_name) : 0;
                fwrite(&a_len, sizeof(a_len), 1, f);
                if (a_len > 0) {
                    fwrite(entry->value.as.array_name, 1, a_len, f);
                }
            } else if (entry->value.type == VAL_FIELD_STRING) {
                fwrite(&entry->value.as.field_str.channel, sizeof(int), 1, f);
                fwrite(&entry->value.as.field_str.offset, sizeof(int), 1, f);
                fwrite(&entry->value.as.field_str.length, sizeof(int), 1, f);
            }
            entry = entry->next;
        }
    }
    return true;
}

bool var_deserialize(VariableContext *ctx, void *fp) {
    if (!ctx || !fp) return false;
    FILE *f = (FILE *)fp;

    var_clear_all(ctx);

    uint32_t count = 0;
    if (fread(&count, sizeof(count), 1, f) != 1) return false;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t name_len = 0;
        if (fread(&name_len, sizeof(name_len), 1, f) != 1) return false;

        char *name = (char *)malloc(name_len + 1);
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

        BValue val;
        memset(&val, 0, sizeof(val));
        val.type = (ValueType)type;

        if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
            if (fread(&val.as.number, sizeof(double), 1, f) != 1) {
                free(name);
                return false;
            }
        } else if (val.type == VAL_STRING) {
            uint32_t s_len = 0;
            if (fread(&s_len, sizeof(s_len), 1, f) != 1) {
                free(name);
                return false;
            }
            if (s_len > 0) {
                char *s_buf = (char *)malloc(s_len + 1);
                if (!s_buf) { free(name); return false; }
                if (fread(s_buf, 1, s_len, f) != s_len) {
                    free(s_buf);
                    free(name);
                    return false;
                }
                s_buf[s_len] = '\0';
                val.as.string = str_create(ctx->str, s_buf, s_len);
                free(s_buf);
            } else {
                val.as.string = NULL;
            }
        } else if (val.type == VAL_ARRAY_REF) {
            uint32_t a_len = 0;
            if (fread(&a_len, sizeof(a_len), 1, f) != 1) {
                free(name);
                return false;
            }
            if (a_len > 0) {
                char *a_buf = (char *)malloc(a_len + 1);
                if (!a_buf) { free(name); return false; }
                if (fread(a_buf, 1, a_len, f) != a_len) {
                    free(a_buf);
                    free(name);
                    return false;
                }
                a_buf[a_len] = '\0';
                val.as.array_name = bpp_strdup(a_buf);
                free(a_buf);
            } else {
                val.as.array_name = NULL;
            }
        } else if (val.type == VAL_FIELD_STRING) {
            if (fread(&val.as.field_str.channel, sizeof(int), 1, f) != 1 ||
                fread(&val.as.field_str.offset, sizeof(int), 1, f) != 1 ||
                fread(&val.as.field_str.length, sizeof(int), 1, f) != 1) {
                free(name);
                return false;
            }
        }

        VarEntry *new_entry = (VarEntry *)malloc(sizeof(VarEntry));
        if (!new_entry) { free(name); return false; }
        new_entry->name = name;
        new_entry->value = val;

        unsigned int bucket = hash_name(ctx, name);
        new_entry->next = ctx->buckets[bucket];
        ctx->buckets[bucket] = new_entry;
    }
    return true;
}


