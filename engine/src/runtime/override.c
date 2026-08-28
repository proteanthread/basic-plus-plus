// FILENAME: override.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h)
// NEEDS: libcore (memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (override.h, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for override within BASIC++.
//
// ---- Includes ----

#include "runtime/override.h"
#include "types/types.h"
#include "vm/vm.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static BppOverrideTable g_override_table;

static BppOverrideTable *get_override_table(VMContext *vm) {
    (void)vm;
    return &g_override_table;
}

void override_init(VMContext *vm) {
    BppOverrideTable *t = get_override_table(vm);
    if (t) {
        runtime_memset(t, 0, sizeof(BppOverrideTable));
    }
}

void override_clear(VMContext *vm) {
    override_init(vm);
}

bool override_register(VMContext *vm, const char *target_name, long target_line, const char *replacement_sub) {
    if (!target_name) return false;
    BppOverrideTable *t = get_override_table(vm);
    if (!t) return false;

    for (int i = 0; i < t->count; ++i) {
        if (runtime_strcasecmp(t->entries[i].target_name, target_name) == 0) {
            t->entries[i].target_line = target_line;
            if (replacement_sub) {
                runtime_strncpy(t->entries[i].replacement_sub, replacement_sub, sizeof(t->entries[i].replacement_sub) - 1);
                t->entries[i].replacement_sub[sizeof(t->entries[i].replacement_sub) - 1] = '\0';
            } else {
                t->entries[i].replacement_sub[0] = '\0';
            }
            t->entries[i].is_active = true;
            return true;
        }
    }

    if (t->count >= MAX_OVERRIDE_ENTRIES) return false;

    runtime_strncpy(t->entries[t->count].target_name, target_name, sizeof(t->entries[t->count].target_name) - 1);
    t->entries[t->count].target_name[sizeof(t->entries[t->count].target_name) - 1] = '\0';

    t->entries[t->count].target_line = target_line;

    if (replacement_sub) {
        runtime_strncpy(t->entries[t->count].replacement_sub, replacement_sub, sizeof(t->entries[t->count].replacement_sub) - 1);
        t->entries[t->count].replacement_sub[sizeof(t->entries[t->count].replacement_sub) - 1] = '\0';
    } else {
        t->entries[t->count].replacement_sub[0] = '\0';
    }

    t->entries[t->count].is_active = true;
    t->count++;
    return true;
}

const BppOverrideEntry *override_lookup(VMContext *vm, const char *target_name) {
    if (!target_name) return NULL;
    BppOverrideTable *t = get_override_table(vm);
    if (!t) return NULL;

    for (int i = 0; i < t->count; ++i) {
        if (t->entries[i].is_active && runtime_strcasecmp(t->entries[i].target_name, target_name) == 0) {
            return &t->entries[i];
        }
    }
    return NULL;
}
