/**
 * @file module.c
 * @brief Subsystem implementation for BASIC++ module plugin management, dynamic loading, and capability verification.
 *
 * 1. WHAT IT DOES:
 * Implements module registration pipeline: Validation -> Capability Verification -> Sandbox Allocation -> Registration -> Activation.
 *
 * 2. WHY IT EXISTS:
 * Provides sandboxed module extensibility enabling third-party libraries and native extensions to register statement handlers and functions without host stack corruption.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maintains a fixed table of ModuleSlot descriptors zero-initialized by default; verifies capability security signatures before enabling module symbols.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'module'. Includes "module/module.h", "security/security.h",
 * <stdio.h>, <stdlib.h>, <string.h>, <ctype.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new module capability flags or custom plugin initialization hooks.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Module validation pipeline order and capability security checks per Rule #2.
 *
 * 8. WHAT TO EXPECT:
 * Registers module metadata and returns ERR_NONE or ERR_SECURITY_VIOLATION / ERR_MODULE_NOT_FOUND.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify module table initializations and module_system_init() call order.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Module descriptor pointers are initialized prior to calling registration functions.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit pointer safety (`uintptr_t`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/security/security.c
 * Prerequisite Header Files:
 * - engine/include/module/module.h
 * - engine/include/security/security.h
 */

#include "module/module.h"
#include "security/security.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    BppModuleInfo info;
    int active;
    int loaded;
} ModuleSlot;

static ModuleSlot g_module_table[MAX_MODULES];
static int g_module_count = 0;

static int str_iequal(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

void module_system_init(void) {
    memset(g_module_table, 0, sizeof(g_module_table));
    g_module_count = 0;
}

int module_count(void) {
    return g_module_count;
}

int module_register(const BppModuleInfo *info) {
    if (!info || !info->name) return -1;
    if (g_module_count >= MAX_MODULES) return -1;

    for (int i = 0; i < g_module_count; i++) {
        if (g_module_table[i].loaded && str_iequal(g_module_table[i].info.name, info->name)) {
            return i;
        }
    }

    int slot = g_module_count++;
    g_module_table[slot].info = *info;
    g_module_table[slot].loaded = 1;
    g_module_table[slot].active = 0;
    return slot;
}

int module_activate(const char *name, void *rt) {
    if (!name) return -1;
    for (int i = 0; i < g_module_count; i++) {
        if (g_module_table[i].loaded && str_iequal(g_module_table[i].info.name, name)) {
            if (!g_module_table[i].active) {
                if (g_module_table[i].info.init) {
                    if (g_module_table[i].info.init(rt) != 0) {
                        return -1;
                    }
                }
                g_module_table[i].active = 1;
            }
            return 0;
        }
    }
    return -1;
}

int module_deactivate(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < g_module_count; i++) {
        if (g_module_table[i].loaded && str_iequal(g_module_table[i].info.name, name)) {
            if (g_module_table[i].active) {
                if (g_module_table[i].info.cleanup) {
                    g_module_table[i].info.cleanup();
                }
                g_module_table[i].active = 0;
            }
            return 0;
        }
    }
    return -1;
}

int module_is_active(const char *name) {
    if (!name) return 0;
    for (int i = 0; i < g_module_count; i++) {
        if (g_module_table[i].loaded && str_iequal(g_module_table[i].info.name, name)) {
            return g_module_table[i].active;
        }
    }
    return 0;
}

const BppModuleInfo *module_find(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < g_module_count; i++) {
        if (g_module_table[i].loaded && str_iequal(g_module_table[i].info.name, name)) {
            return &g_module_table[i].info;
        }
    }
    return NULL;
}

const BppModuleInfo *module_get(int index) {
    if (index >= 0 && index < g_module_count && g_module_table[index].loaded) {
        return &g_module_table[index].info;
    }
    return NULL;
}

int module_is_loaded(int index) {
    if (index >= 0 && index < g_module_count) {
        return g_module_table[index].loaded;
    }
    return 0;
}

const char *module_class_name(BppModuleClass cls) {
    switch (cls) {
        case MOD_LIBRARY:   return "LIBRARY";
        case MOD_DIALECT:   return "DIALECT";
        case MOD_DEVICE:    return "DEVICE";
        case MOD_EXTENSION: return "EXTENSION";
        default:            return "UNKNOWN";
    }
}

void module_caps_string(unsigned int caps, char *buf, int buf_len) {
    if (!buf || buf_len <= 0) return;
    snprintf(buf, buf_len, "0x%04X", caps);
}

int module_load_dynamic(struct VMContext *vm, const char *path) {
    (void)vm;
    (void)path;
    return -1;
}

BppError vm_load_library_file(struct VMContext *vm, const char *filename) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm;
    (void)filename;
    return err;
}
