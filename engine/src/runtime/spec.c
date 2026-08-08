/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file spec.c
 * @brief Runtime component implementation and public API surface for spec.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for spec.c within the runtime subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file spec.c
 * @brief Dynamic Keyword Specification & Feature Registry manager implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements management of dynamically registered statement/function keywords,
 *   parsing .spec and .yaml descriptors, and registering inline blocks.
 * - Why it exists: Supports custom vocabularies and isolated procedural plugins (Phase 19).
 * - Why it works this way: Custom keywords are registered in the lexer dynamically (assigned IDs >= 1000)
 *   and mapped to SpecObjects.
 */

#include "runtime/spec.h"
#include "runtime/vfs.h"
#include "security/security.h"
#include "device/vdev.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "types/config.h"

static SpecObject spec_registry[MAX_SPECS];
static int        spec_count = 0;

static void safe_strcpy(char *dest, const char *src, size_t dest_max) {
    if (!dest || dest_max == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    size_t i = 0;
    for (i = 0; i < dest_max - 1 && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

void spec_registry_init(void) {
    memset(spec_registry, 0, sizeof(spec_registry));
    spec_count = 0;
}

SpecObject *spec_find_by_name(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < spec_count; i++) {
        if (strcasecmp(spec_registry[i].name, name) == 0) {
            return &spec_registry[i];
        }
    }
    return NULL;
}

SpecObject *spec_find_by_kw_id(BppKeywordId kw_id) {
    for (int i = 0; i < spec_count; i++) {
        if (spec_registry[i].kw_id == kw_id) {
            return &spec_registry[i];
        }
    }
    return NULL;
}

int spec_register_inline(VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level) {
    if (!name || spec_count >= MAX_SPECS) {
        return -1;
    }

    /* Check if already registered */
    SpecObject *existing = spec_find_by_name(name);
    if (existing) {
        return 0; /* Already registered */
    }

    SpecObject *spec = &spec_registry[spec_count];
    memset(spec, 0, sizeof(SpecObject));

    safe_strcpy(spec->name, name, sizeof(spec->name));
    spec->category = cat;

    if (lib_path) {
        safe_strcpy(spec->lib_path, lib_path, sizeof(spec->lib_path));
    }
    if (req_level) {
        safe_strcpy(spec->required_level, req_level, sizeof(spec->required_level));
    } else {
        safe_strcpy(spec->required_level, "STANDARD", sizeof(spec->required_level));
    }

    /* Register keyword with lexer dynamically */
    spec->kw_id = keyword_register_custom(name);
    if (spec->kw_id == KW_NONE) {
        return -1;
    }

    spec_count++;
    return 0;
}

static void trim_spaces(char *str) {
    /* Trim trailing spaces/newlines */
    size_t len = strlen(str);
    while (len > 0 && (isspace((unsigned char)str[len - 1]) || str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
    /* Trim leading spaces */
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static void extract_quoted(const char *src, char *dest, size_t dest_max) {
    const char *quote1 = strchr(src, '"');
    if (quote1) {
        const char *quote2 = strchr(quote1 + 1, '"');
        if (quote2) {
            size_t len = (size_t)(quote2 - (quote1 + 1));
            if (len >= dest_max) len = dest_max - 1;
            memcpy(dest, quote1 + 1, len);
            dest[len] = '\0';
            return;
        }
    }
    /* Fallback: copy without quotes */
    safe_strcpy(dest, src, dest_max);
    trim_spaces(dest);
}

static int parse_spec_yaml(VMContext *vm, const char *resolved_path) {
    FILE *fp = fopen(resolved_path, "r");
    if (!fp) return -1;

    char line[512];
    SpecObject current_spec;
    memset(&current_spec, 0, sizeof(SpecObject));
    strcpy(current_spec.required_level, "STANDARD");

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;

        char *colon = strchr(p, ':');
        if (!colon) continue;

        *colon = '\0';
        char key[64];
        safe_strcpy(key, p, sizeof(key));
        trim_spaces(key);

        char val[256];
        extract_quoted(colon + 1, val, sizeof(val));

        if (strcasecmp(key, "name") == 0) {
            safe_strcpy(current_spec.name, val, sizeof(current_spec.name));
        } else if (strcasecmp(key, "version") == 0) {
            safe_strcpy(current_spec.version, val, sizeof(current_spec.version));
        } else if (strcasecmp(key, "category") == 0) {
            if (strcasecmp(val, "STATEMENT") == 0) current_spec.category = SPEC_CAT_STATEMENT;
            else if (strcasecmp(val, "FUNCTION") == 0) current_spec.category = SPEC_CAT_FUNCTION;
        } else if (strcasecmp(key, "lib") == 0 || strcasecmp(key, "lib_path") == 0) {
            safe_strcpy(current_spec.lib_path, val, sizeof(current_spec.lib_path));
        } else if (strcasecmp(key, "security") == 0 || strcasecmp(key, "required_level") == 0) {
            safe_strcpy(current_spec.required_level, val, sizeof(current_spec.required_level));
        }
    }
    fclose(fp);

    if (current_spec.name[0] != '\0') {
        return spec_register_inline(vm, current_spec.name, current_spec.category, current_spec.lib_path, current_spec.required_level);
    }
    return -1;
}

static int parse_spec_block(VMContext *vm, const char *resolved_path) {
    FILE *fp = fopen(resolved_path, "r");
    if (!fp) return -1;

    char line[512];
    SpecObject current_spec;
    int in_spec = 0;
    memset(&current_spec, 0, sizeof(SpecObject));
    strcpy(current_spec.required_level, "STANDARD");

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;

        if (strncmp(p, "DEFINE SPECIFICATION", 20) == 0) {
            char name_val[64];
            extract_quoted(p + 20, name_val, sizeof(name_val));
            safe_strcpy(current_spec.name, name_val, sizeof(current_spec.name));
            in_spec = 1;
        } else if (in_spec && strncmp(p, "END SPECIFICATION", 17) == 0) {
            spec_register_inline(vm, current_spec.name, current_spec.category, current_spec.lib_path, current_spec.required_level);
            in_spec = 0;
            memset(&current_spec, 0, sizeof(SpecObject));
            strcpy(current_spec.required_level, "STANDARD");
        } else if (in_spec) {
            if (strncmp(p, "CATEGORY", 8) == 0) {
                char val[64];
                extract_quoted(p + 8, val, sizeof(val));
                if (strcasecmp(val, "STATEMENT") == 0) current_spec.category = SPEC_CAT_STATEMENT;
                else if (strcasecmp(val, "FUNCTION") == 0) current_spec.category = SPEC_CAT_FUNCTION;
            } else if (strncmp(p, "VERSION", 7) == 0) {
                char val[64];
                extract_quoted(p + 7, val, sizeof(val));
                safe_strcpy(current_spec.version, val, sizeof(current_spec.version));
            } else if (strncmp(p, "LIB", 3) == 0) {
                char val[256];
                extract_quoted(p + 3, val, sizeof(val));
                safe_strcpy(current_spec.lib_path, val, sizeof(current_spec.lib_path));
            } else if (strncmp(p, "SECURITY", 8) == 0) {
                char val[64];
                extract_quoted(p + 8, val, sizeof(val));
                safe_strcpy(current_spec.required_level, val, sizeof(current_spec.required_level));
            }
        }
    }
    fclose(fp);
    return 0;
}

int spec_load_file(VMContext *vm, const char *filename) {
    char resolved_path[512];
    vfs_resolve(vm_get_vfs(vm), filename, resolved_path, sizeof(resolved_path));

    /* Verify path sandbox read access */
    if (security_check_file_path(resolved_path, 0) != 0) {
        return -2; /* Permission denied */
    }

    int parse_res = -1;
    /* Check file extension */
    const char *ext = strrchr(resolved_path, '.');
    if (ext && (strcasecmp(ext, ".yaml") == 0 || strcasecmp(ext, ".yml") == 0)) {
        parse_res = parse_spec_yaml(vm, resolved_path);
    } else {
        parse_res = parse_spec_block(vm, resolved_path);
    }

    if (parse_res == 0) {
        /* Extract directory part of the spec file to load companion libs relative to it */
        char dir_part[512] = {0};
        const char *last_slash = strrchr(resolved_path, '/');
        const char *last_backslash = strrchr(resolved_path, '\\');
        const char *sep = (last_slash > last_backslash) ? last_slash : last_backslash;
        if (sep) {
            size_t dir_len = (size_t)(sep - resolved_path + 1);
            if (dir_len < sizeof(dir_part)) {
                memcpy(dir_part, resolved_path, dir_len);
                dir_part[dir_len] = '\0';
            }
        }
        spec_load_companion_libraries(vm, dir_part);
    }
    return parse_res;
}

void spec_list_all(VMContext *vm) {
    VDevContext *vdev = vm_get_vdev(vm);
    if (spec_count == 0) {
        vdev_printf(vdev, "No dynamically registered specifications.\n");
        return;
    }
    vdev_printf(vdev, "Loaded Dynamic Specifications:\n");
    vdev_printf(vdev, "%-16s %-10s %-8s %-12s %s\n", "Keyword", "Category", "Version", "Security", "Companion Lib");
    vdev_printf(vdev, "%-16s %-10s %-8s %-12s %s\n", "-------", "--------", "-------", "--------", "-------------");
    for (int i = 0; i < spec_count; i++) {
        vdev_printf(vdev, "%-16s %-10s %-8s %-12s %s\n",
                    spec_registry[i].name,
                    spec_registry[i].category == SPEC_CAT_STATEMENT ? "STATEMENT" : "FUNCTION",
                    spec_registry[i].version[0] ? spec_registry[i].version : "1.0",
                    spec_registry[i].required_level,
                    spec_registry[i].lib_path[0] ? spec_registry[i].lib_path : "None");
    }
}

extern BppError vm_load_library_file(VMContext *vm, const char *filename);

int spec_load_companion_libraries(VMContext *vm, const char *dir_part) {
    int loaded_count = 0;
    for (int i = 0; i < spec_count; i++) {
        if (spec_registry[i].lib_path[0] != '\0' && !spec_registry[i].lib_loaded) {
            char full_path[1024];
            if (dir_part && dir_part[0] != '\0') {
                snprintf(full_path, sizeof(full_path), "%s%s", dir_part, spec_registry[i].lib_path);
            } else {
                safe_strcpy(full_path, spec_registry[i].lib_path, sizeof(full_path));
            }
            
            BppError err = vm_load_library_file(vm, full_path);
            if (err.code == 0) {
                spec_registry[i].lib_loaded = true;
                loaded_count++;
            } else {
                /* Try to load directly from the current directory as fallback */
                err = vm_load_library_file(vm, spec_registry[i].lib_path);
                if (err.code == 0) {
                    spec_registry[i].lib_loaded = true;
                    loaded_count++;
                } else {
                    VDevContext *vdev = vm_get_vdev(vm);
                    vdev_printf(vdev, "Warning: Failed to load companion library: %s\n", spec_registry[i].lib_path);
                }
            }
        }
    }
    return loaded_count;
}

int spec_get_count(void) {
    return spec_count;
}

SpecObject *spec_get_by_index(int idx) {
    if (idx < 0 || idx >= spec_count) {
        return NULL;
    }
    return &spec_registry[idx];
}
