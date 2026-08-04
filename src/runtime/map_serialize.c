/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file map_serialize.c
 * @brief JSON, XML, YAML, and INI serialization / deserialization converters for BppMap.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements parser and stringifier logic for JSON, XML, YAML, and INI formats.
 * - Why it exists: Provides dynamic data conversion for system communication and configuration formats.
 * - Why it works this way: It uses robust string-parsing loops to extract keys and values without
 *   external library dependencies, ensuring 100% portability to any host system.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Parsing grammar rules, escaping sequences.
 * - What cannot be changed: API signatures for stringifier and parser functions.
 * - What to expect: Parsers return a new BppMap (with ref_count = 1). Stringifiers return dynamic allocated C strings.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Input strings are null-terminated. Output strings are dynamically allocated via malloc.
 * - Portability concerns: None. ANSI/ISO C17 compliant.
 */

#include "bpp_map.h"
#include "bpp_strings.h"
#include "bpp_config.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>


#if BPP_SUPPORT_JSON
static void stringify_json_internal(BppMap *map, char **p_buf, size_t *p_cap, size_t *p_len);
#endif

/* Helper: skip whitespace */
static const char *skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

#if BPP_SUPPORT_INI
static const char *skip_inline_ws(const char *p) {
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return p;
}
#endif

/* Safe realloc helper. Doubles the buffer capacity.
 * On failure, the original buffer is preserved and false is returned.
 * On success, *p_buf and *p_cap are updated and true is returned. */
static bool safe_buf_grow(char **p_buf, size_t *p_cap) {
    size_t new_cap = *p_cap * 2;
    char *new_buf = (char *)realloc(*p_buf, new_cap);
    if (!new_buf) return false;
    *p_buf = new_buf;
    *p_cap = new_cap;
    return true;
}

static void buf_append_str(char **p_buf, size_t *p_cap, size_t *p_len, const char *str) {
    size_t len = strlen(str);
    while (*p_len + len + 1 >= *p_cap) {
        if (!safe_buf_grow(p_buf, p_cap)) return;
    }
    memcpy(*p_buf + *p_len, str, len + 1);
    *p_len += len;
}

static void buf_append_char(char **p_buf, size_t *p_cap, size_t *p_len, char c) {
    if (*p_len + 2 >= *p_cap) {
        if (!safe_buf_grow(p_buf, p_cap)) return;
    }
    (*p_buf)[(*p_len)++] = c;
    (*p_buf)[*p_len] = '\0';
}

#if BPP_SUPPORT_JSON

/* Helper: parse a JSON string token */
static char *parse_json_string(const char **p_in) {
    const char *p = *p_in;
    if (*p != '"') return NULL;
    p++; /* skip '"' */
    const char *start = p;
    while (*p && *p != '"') p++;
    size_t len = p - start;
    char *res = (char *)calloc(1, len + 1);
    if (res) {
        memcpy(res, start, len);
        res[len] = '\0';
    }
    if (*p == '"') p++;
    *p_in = p;
    return res;
}

/* Helper: parse a JSON value (number, string, nested map) */
static BValue parse_json_value(void *str_ctx, const char **p_in, bool *ok) {
    BValue val;
    val.type = VAL_NONE;
    val.as.number = 0.0;
    *ok = false;

    const char *p = skip_ws(*p_in);
    if (*p == '"') {
        char *s = parse_json_string(&p);
        if (s) {
            val.type = VAL_STRING;
            val.as.string = str_create((StringContext *)str_ctx, s, strlen(s));
            free(s);
            *ok = true;
        }
    } else if (*p == '{') {
        p++; /* skip '{' */
        BppMap *sub = bpp_map_create();
        if (sub) {
            p = skip_ws(p);
            if (*p == '}') {
                p++;
                val.type = VAL_MAP;
                val.as.map = sub;
                *ok = true;
            } else {
                while (*p) {
                    p = skip_ws(p);
                    char *k = parse_json_string(&p);
                    if (!k) break;
                    p = skip_ws(p);
                    if (*p != ':') {
                        free(k);
                        break;
                    }
                    p++; /* skip ':' */
                    p = skip_ws(p);
                    bool val_ok = false;
                    BValue v = parse_json_value(str_ctx, &p, &val_ok);
                    if (!val_ok) {
                        free(k);
                        break;
                    }
                    bpp_map_set(str_ctx, sub, k, v);
                    if (v.type == VAL_STRING && v.as.string) {
                        str_release((StringContext *)str_ctx, v.as.string);
                    } else if (v.type == VAL_MAP && v.as.map) {
                        bpp_map_release(str_ctx, v.as.map);
                    }
                    free(k);
                    p = skip_ws(p);
                    if (*p == ',') {
                        p++;
                    } else if (*p == '}') {
                        p++;
                        val.type = VAL_MAP;
                        val.as.map = sub;
                        *ok = true;
                        break;
                    } else {
                        break;
                    }
                }
            }
        }
        if (!*ok && sub) {
            bpp_map_release(str_ctx, sub);
        }
    } else if (isdigit((unsigned char)*p) || *p == '-' || *p == '+') {
        char *endptr;
        double d = strtod(p, &endptr);
        if (endptr != p) {
            val.type = VAL_NUMBER;
            val.as.number = d;
            p = endptr;
            *ok = true;
        }
    } else if (strncmp(p, "true", 4) == 0) {
        val.type = VAL_NUMBER;
        val.as.number = 1.0;
        p += 4;
        *ok = true;
    } else if (strncmp(p, "false", 5) == 0) {
        val.type = VAL_NUMBER;
        val.as.number = 0.0;
        p += 5;
        *ok = true;
    } else if (strncmp(p, "null", 4) == 0) {
        val.type = VAL_NONE;
        p += 4;
        *ok = true;
    }

    *p_in = p;
    return val;
}

BppMap *bpp_map_parse_json(void *str_ctx, const char *json) {
    if (!json) return NULL;
    const char *p = json;
    bool ok = false;
    BValue val = parse_json_value(str_ctx, &p, &ok);
    if (ok && val.type == VAL_MAP) {
        return val.as.map;
    }
    if (val.type == VAL_STRING && val.as.string) {
        str_release((StringContext *)str_ctx, val.as.string);
    } else if (val.type == VAL_MAP && val.as.map) {
        bpp_map_release(str_ctx, val.as.map);
    }
    return NULL;
}

static void stringify_json_internal(BppMap *map, char **p_buf, size_t *p_cap, size_t *p_len) {
    if (!map) return;

    buf_append_char(p_buf, p_cap, p_len, '{');

    for (int i = 0; i < map->count; ++i) {
        if (i > 0) {
            buf_append_char(p_buf, p_cap, p_len, ',');
        }

        /* Format key */
        char k_fmt[512];
        snprintf(k_fmt, sizeof(k_fmt), "\"%s\":", map->entries[i].key);
        buf_append_str(p_buf, p_cap, p_len, k_fmt);

        /* Format value */
        BValue val = map->entries[i].val;
        if (val.type == VAL_NUMBER) {
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%.16g", val.as.number);
            buf_append_str(p_buf, p_cap, p_len, num_buf);
        } else if (val.type == VAL_STRING) {
            const char *str = val.as.string ? str_data(val.as.string) : "";
            buf_append_char(p_buf, p_cap, p_len, '"');
            buf_append_str(p_buf, p_cap, p_len, str);
            buf_append_char(p_buf, p_cap, p_len, '"');
        } else if (val.type == VAL_MAP) {
            stringify_json_internal(val.as.map, p_buf, p_cap, p_len);
        } else {
            buf_append_str(p_buf, p_cap, p_len, "null");
        }
    }

    buf_append_char(p_buf, p_cap, p_len, '}');
}

char *bpp_map_stringify_json(BppMap *map) {
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)calloc(1, cap);
    if (!buf) return NULL;
    buf[0] = '\0';
    stringify_json_internal(map, &buf, &cap, &len);
    return buf;
}
#else
BppMap *bpp_map_parse_json(void *str_ctx, const char *json) {
    (void)str_ctx; (void)json;
    return NULL;
}
char *bpp_map_stringify_json(BppMap *map) {
    (void)map;
    return NULL;
}
#endif

/* XML Parser & Stringifier */
#if BPP_SUPPORT_XML
BppMap *bpp_map_parse_xml(void *str_ctx, const char *xml) {
    BppMap *map = bpp_map_create();
    if (!xml || !map) return map;

    const char *p = xml;
    while (*p) {
        p = skip_ws(p);
        if (*p == '<') {
            if (*(p + 1) == '/') {
                /* End tag: skip */
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
                continue;
            }
            p++; /* skip '<' */
            const char *t_start = p;
            while (*p && *p != '>') p++;
            size_t t_len = p - t_start;
            char tag[256];
            if (t_len >= sizeof(tag)) t_len = sizeof(tag) - 1;
            memcpy(tag, t_start, t_len);
            tag[t_len] = '\0';
            if (*p == '>') p++;

            /* Parse contents until next '<' */
            const char *val_start = p;
            while (*p && *p != '<') p++;
            size_t val_len = p - val_start;
            char *val_str = (char *)calloc(1, val_len + 1);
            if (val_str) {
                memcpy(val_str, val_start, val_len);
                val_str[val_len] = '\0';
            }

            /* If it's a nested XML element (contains '<' before end tag) */
            p = skip_ws(p);
            if (*p == '<' && *(p + 1) != '/') {
                /* Nested child tag - parse recursively */
                free(val_str);
                BppMap *sub = bpp_map_parse_xml(str_ctx, val_start);
                BValue val;
                val.type = VAL_MAP;
                val.as.map = sub;
                bpp_map_set(str_ctx, map, tag, val);
                bpp_map_release(str_ctx, sub);
            } else {
                /* Simple text content */
                BValue val;
                /* Try to parse as number first */
                char *endptr;
                double d = strtod(val_str, &endptr);
                if (endptr != val_str && *skip_ws(endptr) == '\0') {
                    val.type = VAL_NUMBER;
                    val.as.number = d;
                } else {
                    val.type = VAL_STRING;
                    val.as.string = str_create((StringContext *)str_ctx, val_str, strlen(val_str));
                }
                bpp_map_set(str_ctx, map, tag, val);
                if (val.type == VAL_STRING && val.as.string) {
                    str_release((StringContext *)str_ctx, val.as.string);
                }
                free(val_str);
            }

            /* Consume close tag </tag> */
            if (*p == '<' && *(p + 1) == '/') {
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
            }
        } else {
            p++;
        }
    }
    return map;
}

static void stringify_xml_internal(BppMap *map, char **p_buf, size_t *p_cap, size_t *p_len) {
    if (!map) return;
    for (int i = 0; i < map->count; ++i) {
        char tag[256];
        snprintf(tag, sizeof(tag), "%s", map->entries[i].key);

        /* Open tag */
        buf_append_char(p_buf, p_cap, p_len, '<');
        buf_append_str(p_buf, p_cap, p_len, tag);
        buf_append_char(p_buf, p_cap, p_len, '>');

        /* Value */
        BValue val = map->entries[i].val;
        if (val.type == VAL_NUMBER) {
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%.16g", val.as.number);
            buf_append_str(p_buf, p_cap, p_len, num_buf);
        } else if (val.type == VAL_STRING) {
            const char *str = val.as.string ? str_data(val.as.string) : "";
            buf_append_str(p_buf, p_cap, p_len, str);
        } else if (val.type == VAL_MAP) {
            stringify_xml_internal(val.as.map, p_buf, p_cap, p_len);
        }

        /* Close tag */
        buf_append_char(p_buf, p_cap, p_len, '<');
        buf_append_char(p_buf, p_cap, p_len, '/');
        buf_append_str(p_buf, p_cap, p_len, tag);
        buf_append_char(p_buf, p_cap, p_len, '>');
    }
}

char *bpp_map_stringify_xml(BppMap *map) {
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)calloc(1, cap);
    if (!buf) return NULL;
    buf[0] = '\0';
    stringify_xml_internal(map, &buf, &cap, &len);
    return buf;
}
#else
BppMap *bpp_map_parse_xml(void *str_ctx, const char *xml) {
    (void)str_ctx; (void)xml;
    return bpp_map_create();
}
char *bpp_map_stringify_xml(BppMap *map) {
    (void)map;
    return NULL;
}
#endif

/* YAML Parser & Stringifier */
BppMap *bpp_map_parse_yaml(void *str_ctx, const char *yaml) {
    BppMap *map = bpp_map_create();
    if (!yaml || !map) return map;

    const char *p = yaml;
    while (*p) {
        /* Parse line-by-line */
        p = skip_ws(p);
        if (*p == '\0') break;

        const char *k_start = p;
        while (*p && *p != ':' && *p != '\n') p++;
        if (*p == ':') {
            size_t k_len = p - k_start;
            char tag[256];
            if (k_len >= sizeof(tag)) k_len = sizeof(tag) - 1;
            memcpy(tag, k_start, k_len);
            tag[k_len] = '\0';
            p++; /* skip ':' */

            p = skip_ws(p);
            const char *val_start = p;
            while (*p && *p != '\n') p++;
            const char *val_end = p - 1;
            while (val_end > val_start && isspace((unsigned char)*val_end)) val_end--;
            size_t val_len = (val_end >= val_start) ? (val_end - val_start + 1) : 0;

            char *val_str = (char *)calloc(1, val_len + 1);
            if (val_str) {
                memcpy(val_str, val_start, val_len);
                val_str[val_len] = '\0';
            }

            BValue val;
            char *endptr;
            double d = strtod(val_str, &endptr);
            if (endptr != val_str && *skip_ws(endptr) == '\0') {
                val.type = VAL_NUMBER;
                val.as.number = d;
            } else {
                val.type = VAL_STRING;
                val.as.string = str_create((StringContext *)str_ctx, val_str, strlen(val_str));
            }
            bpp_map_set(str_ctx, map, tag, val);
            if (val.type == VAL_STRING && val.as.string) {
                str_release((StringContext *)str_ctx, val.as.string);
            }
            free(val_str);
        }
        if (*p == '\n') p++;
    }
    return map;
}

static void stringify_yaml_internal(BppMap *map, char **p_buf, size_t *p_cap, size_t *p_len, int indent) {
    if (!map) return;
    for (int i = 0; i < map->count; ++i) {
        /* Add indentation spaces */
        for (int s = 0; s < indent; ++s) {
            buf_append_char(p_buf, p_cap, p_len, ' ');
        }

        /* Format key */
        char k_fmt[256];
        snprintf(k_fmt, sizeof(k_fmt), "%s: ", map->entries[i].key);
        buf_append_str(p_buf, p_cap, p_len, k_fmt);

        BValue val = map->entries[i].val;
        if (val.type == VAL_NUMBER) {
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%.16g\n", val.as.number);
            buf_append_str(p_buf, p_cap, p_len, num_buf);
        } else if (val.type == VAL_STRING) {
            const char *str = val.as.string ? str_data(val.as.string) : "";
            buf_append_str(p_buf, p_cap, p_len, str);
            buf_append_char(p_buf, p_cap, p_len, '\n');
        } else if (val.type == VAL_MAP) {
            buf_append_char(p_buf, p_cap, p_len, '\n');
            stringify_yaml_internal(val.as.map, p_buf, p_cap, p_len, indent + 2);
        }
    }
}

char *bpp_map_stringify_yaml(BppMap *map) {
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)calloc(1, cap);
    if (!buf) return NULL;
    buf[0] = '\0';
    stringify_yaml_internal(map, &buf, &cap, &len, 0);
    return buf;
}

/* INI Parser & Stringifier */
#if BPP_SUPPORT_INI
BppMap *bpp_map_parse_ini(void *str_ctx, const char *ini) {
    BppMap *map = bpp_map_create();
    if (!ini || !map) return map;

    const char *p = ini;
    BppMap *current_section = map;

    while (*p) {
        p = skip_ws(p);
        if (*p == '\0') break;

        /* Check for comment */
        if (*p == ';' || *p == '#') {
            while (*p && *p != '\n') p++;
            continue;
        }

        /* Check for section: [section] */
        if (*p == '[') {
            p++; /* skip '[' */
            const char *s_start = p;
            while (*p && *p != ']' && *p != '\n') p++;
            if (*p == ']') {
                size_t s_len = p - s_start;
                char sect[256];
                if (s_len >= sizeof(sect)) s_len = sizeof(sect) - 1;
                memcpy(sect, s_start, s_len);
                sect[s_len] = '\0';
                p++; /* skip ']' */

                /* Create section map if not exists */
                BValue val;
                if (bpp_map_get(map, sect, &val) && val.type == VAL_MAP) {
                    current_section = val.as.map;
                } else {
                    BppMap *sub = bpp_map_create();
                    val.type = VAL_MAP;
                    val.as.map = sub;
                    bpp_map_set(str_ctx, map, sect, val);
                    bpp_map_release(str_ctx, sub);
                    current_section = sub;
                }
            }
            continue;
        }

        /* Parse key = value */
        const char *k_start = p;
        while (*p && *p != '=' && *p != '\n') p++;
        if (*p == '=') {
            /* Trim trailing whitespace from key */
            const char *k_end = p - 1;
            while (k_end > k_start && isspace((unsigned char)*k_end)) k_end--;
            size_t k_len = k_end - k_start + 1;
            char tag[256];
            if (k_len >= sizeof(tag)) k_len = sizeof(tag) - 1;
            memcpy(tag, k_start, k_len);
            tag[k_len] = '\0';

            p++; /* skip '=' */
            p = skip_inline_ws(p);
            const char *val_start = p;
            while (*p && *p != '\n') p++;
            /* Trim trailing whitespace from value */
            const char *val_end = p - 1;
            while (val_end > val_start && isspace((unsigned char)*val_end)) val_end--;
            size_t val_len = (val_end >= val_start) ? (val_end - val_start + 1) : 0;

            char *val_str = (char *)calloc(1, val_len + 1);
            if (val_str) {
                memcpy(val_str, val_start, val_len);
                val_str[val_len] = '\0';
            }

            BValue val;
            char *endptr;
            double d = strtod(val_str, &endptr);
            if (endptr != val_str && *skip_ws(endptr) == '\0') {
                val.type = VAL_NUMBER;
                val.as.number = d;
            } else {
                val.type = VAL_STRING;
                val.as.string = str_create((StringContext *)str_ctx, val_str, strlen(val_str));
            }
            bpp_map_set(str_ctx, current_section, tag, val);
            if (val.type == VAL_STRING && val.as.string) {
                str_release((StringContext *)str_ctx, val.as.string);
            }
            free(val_str);
        }
        if (*p == '\n') p++;
    }
    return map;
}

static void stringify_ini_internal(BppMap *map, char **p_buf, size_t *p_cap, size_t *p_len) {
    if (!map) return;

    /* First pass: print all simple keys */
    for (int i = 0; i < map->count; ++i) {
        BValue val = map->entries[i].val;
        if (val.type != VAL_MAP) {
            char k_fmt[256];
            snprintf(k_fmt, sizeof(k_fmt), "%s = ", map->entries[i].key);
            buf_append_str(p_buf, p_cap, p_len, k_fmt);

            if (val.type == VAL_NUMBER) {
                char num_buf[64];
                snprintf(num_buf, sizeof(num_buf), "%.16g\n", val.as.number);
                buf_append_str(p_buf, p_cap, p_len, num_buf);
            } else if (val.type == VAL_STRING) {
                const char *str = val.as.string ? str_data(val.as.string) : "";
                buf_append_str(p_buf, p_cap, p_len, str);
                buf_append_char(p_buf, p_cap, p_len, '\n');
            }
        }
    }

    /* Second pass: print sections */
    for (int i = 0; i < map->count; ++i) {
        BValue val = map->entries[i].val;
        if (val.type == VAL_MAP) {
            char s_hdr[512];
            snprintf(s_hdr, sizeof(s_hdr), "\n[%s]\n", map->entries[i].key);
            buf_append_str(p_buf, p_cap, p_len, s_hdr);

            /* Print sub-keys */
            BppMap *sub = val.as.map;
            for (int j = 0; j < sub->count; ++j) {
                char k_fmt[256];
                snprintf(k_fmt, sizeof(k_fmt), "%s = ", sub->entries[j].key);
                buf_append_str(p_buf, p_cap, p_len, k_fmt);

                BValue sub_val = sub->entries[j].val;
                if (sub_val.type == VAL_NUMBER) {
                    char num_buf[64];
                    snprintf(num_buf, sizeof(num_buf), "%.16g\n", sub_val.as.number);
                    buf_append_str(p_buf, p_cap, p_len, num_buf);
                } else if (sub_val.type == VAL_STRING) {
                    const char *str = sub_val.as.string ? str_data(sub_val.as.string) : "";
                    buf_append_str(p_buf, p_cap, p_len, str);
                    buf_append_char(p_buf, p_cap, p_len, '\n');
                }
            }
        }
    }
}

char *bpp_map_stringify_ini(BppMap *map) {
    size_t cap = 256;
    size_t len = 0;
    char *buf = (char *)calloc(1, cap);
    if (!buf) return NULL;
    buf[0] = '\0';
    stringify_ini_internal(map, &buf, &cap, &len);
    return buf;
}
#else
BppMap *bpp_map_parse_ini(void *str_ctx, const char *ini) {
    (void)str_ctx; (void)ini;
    return bpp_map_create();
}
char *bpp_map_stringify_ini(BppMap *map) {
    (void)map;
    return NULL;
}
#endif
