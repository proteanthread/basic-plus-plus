// FILENAME: module.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, help.c, system.c)
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c, module.h, string.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c)
// Provides core logic and interface definitions for module within BASIC++.
//
// ---- Includes ----

#include "module/module.h"
#include "security/security.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "types/errors.h"
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
    if (!vm || !filename) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }
    MemoryContext *mem = vm_get_mem(vm);
    char line_buf[1024];
    BppLineNumber auto_line = 60000;
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        char *p = line_buf;
        while (isspace((unsigned char)*p)) p++;
        if (*p != '\0') {
            if (isdigit((unsigned char)*p)) {
                BppLineNumber line_num = (BppLineNumber)atof(p);
                while (isdigit((unsigned char)*p) || *p == '.') p++;
                while (isspace((unsigned char)*p)) p++;
                mem_lib_program_insert(mem, line_num, p);
            } else {
                mem_lib_program_insert(mem, auto_line, p);
                auto_line += 10;
            }
        }
    }
    fclose(fp);
    return err;
}
