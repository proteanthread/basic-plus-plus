// FILENAME: language_descriptor.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex, libscript, libstandard, libadvanced
// NEEDS: language_descriptor.h, strops.h, memops.h
// Implements core runtime registry and reflection for language descriptors.

#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"

//
// ---- Static State ----

static LanguageDescriptor *g_registry = NULL;
static int                 g_count = 0;
static int                 g_capacity = 0;

//
// ---- Static Helpers ----

static int strcasecmp_portable(const char *s1, const char *s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    return runtime_strcasecmp(s1, s2);
}

//
// ---- Public Registry API ----

void lang_desc_init(void) {
    if (g_registry) {
        runtime_free(g_registry);
        g_registry = NULL;
    }
    g_count = 0;
    g_capacity = LANG_DESC_INITIAL_CAPACITY;
    g_registry = (LanguageDescriptor *)runtime_calloc((size_t)g_capacity, sizeof(LanguageDescriptor));
}

void lang_desc_shutdown(void) {
    if (g_registry) {
        runtime_free(g_registry);
        g_registry = NULL;
    }
    g_count = 0;
    g_capacity = 0;
}

int lang_desc_register(const LanguageDescriptor *desc) {
    if (!desc || !desc->name) return -1;

    if (!g_registry) {
        lang_desc_init();
        if (!g_registry) return -1;
    }

    // Check for existing registration to update in-place
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].name && strcasecmp_portable(g_registry[i].name, desc->name) == 0) {
            g_registry[i] = *desc;
            return i;
        }
    }

    // Dynamic capacity expansion using safe runtime_realloc pattern
    if (g_count >= g_capacity) {
        int new_capacity = (g_capacity > 0) ? (g_capacity * 2) : LANG_DESC_INITIAL_CAPACITY;
        LanguageDescriptor *new_buf = (LanguageDescriptor *)runtime_realloc(
            g_registry, (size_t)new_capacity * sizeof(LanguageDescriptor));
        if (!new_buf) {
            return -1;
        }
        runtime_memset(new_buf + g_capacity, 0, (size_t)(new_capacity - g_capacity) * sizeof(LanguageDescriptor));
        g_registry = new_buf;
        g_capacity = new_capacity;
    }

    g_registry[g_count] = *desc;
    return g_count++;
}

int lang_desc_override(const char *name, const LanguageDescriptor *desc) {
    if (!name || !desc || !g_registry) return -1;
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].name && strcasecmp_portable(g_registry[i].name, name) == 0) {
            g_registry[i] = *desc;
            return i;
        }
    }
    return lang_desc_register(desc);
}

const LanguageDescriptor *lang_desc_find(const char *name) {
    if (!name || !g_registry) return NULL;
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].name && strcasecmp_portable(g_registry[i].name, name) == 0) {
            return &g_registry[i];
        }
    }

    // Try matching without trailing sigils ($, %, &, !, #)
    size_t nlen = runtime_strlen(name);
    if (nlen > 1 && (name[nlen - 1] == '$' || name[nlen - 1] == '%' || name[nlen - 1] == '&' ||
                     name[nlen - 1] == '!' || name[nlen - 1] == '#')) {
        for (int i = 0; i < g_count; ++i) {
            if (g_registry[i].name && runtime_strncasecmp(g_registry[i].name, name, nlen - 1) == 0 &&
                runtime_strlen(g_registry[i].name) == nlen - 1) {
                return &g_registry[i];
            }
        }
    }

    // Try matching prefix in syntax signature with word boundary
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].syntax) {
            const char *syn = g_registry[i].syntax;
            if (runtime_strncasecmp(syn, name, nlen) == 0) {
                char c = syn[nlen];
                if (c == '\0' || c == ' ' || c == '\t' || c == '(' || c == '[' || c == '$' || c == '%' || c == '&' || c == '#' || c == '!') {
                    return &g_registry[i];
                }
            }
        }
    }

    return NULL;
}

const LanguageDescriptor *lang_desc_get(int index) {
    if (!g_registry || index < 0 || index >= g_count) return NULL;
    return &g_registry[index];
}

int lang_desc_count(void) {
    return g_count;
}

int lang_desc_query_category(const char *category, const LanguageDescriptor **results, int max_results) {
    if (!category || !results || max_results <= 0 || !g_registry) return 0;
    int found = 0;
    for (int i = 0; i < g_count && found < max_results; ++i) {
        if (g_registry[i].category && strcasecmp_portable(g_registry[i].category, category) == 0) {
            results[found++] = &g_registry[i];
        }
    }
    return found;
}

int lang_desc_get_categories(char out_categories[][64], int max_categories) {
    if (!out_categories || max_categories <= 0 || !g_registry) return 0;
    int cat_count = 0;

    for (int i = 0; i < g_count && cat_count < max_categories; ++i) {
        const char *c = g_registry[i].category;
        if (!c || c[0] == '\0') continue;

        bool exists = false;
        for (int j = 0; j < cat_count; ++j) {
            if (strcasecmp_portable(out_categories[j], c) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            runtime_strncpy(out_categories[cat_count], c, 63);
            out_categories[cat_count][63] = '\0';
            cat_count++;
        }
    }
    return cat_count;
}

const char *lang_desc_lookup_error_msg(int code) {
    if (!g_registry || code <= 0) return NULL;
    for (int i = 0; i < g_count; ++i) {
        if (g_registry[i].errors && g_registry[i].error_count > 0) {
            for (size_t e = 0; e < g_registry[i].error_count; ++e) {
                if (g_registry[i].errors[e].code == code && g_registry[i].errors[e].description) {
                    return g_registry[i].errors[e].description;
                }
            }
        }
    }
    return NULL;
}

char *lang_desc_export_json(void) {
    if (!g_registry || g_count == 0) return NULL;
    size_t buf_size = (size_t)g_count * 512 + 64;
    char *json = (char *)runtime_calloc(1, buf_size);
    if (!json) return NULL;

    runtime_strncpy(json, "[\n", buf_size - 1);
    size_t offset = runtime_strlen(json);

    for (int i = 0; i < g_count; ++i) {
        char item[512];
        runtime_snprintf(item, sizeof(item),
            "  {\"name\": \"%s\", \"category\": \"%s\", \"syntax\": \"%s\", \"subsystem\": %u}%s\n",
            g_registry[i].name ? g_registry[i].name : "",
            g_registry[i].category ? g_registry[i].category : "",
            g_registry[i].syntax ? g_registry[i].syntax : "",
            g_registry[i].subsystem,
            (i < g_count - 1) ? "," : "");

        size_t item_len = runtime_strlen(item);
        if (offset + item_len < buf_size - 4) {
            runtime_memcpy(json + offset, item, item_len);
            offset += item_len;
            json[offset] = '\0';
        }
    }

    if (offset < buf_size - 3) {
        runtime_strncat(json, "]\n", buf_size - offset - 1);
    }

    return json;
}

void lang_desc_free_json(char *json) {
    if (json) {
        runtime_free(json);
    }
}
