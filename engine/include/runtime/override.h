// FILENAME: override.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, override.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for override within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_OVERRIDE_H
#define RUNTIME_OVERRIDE_H

#include "types/types.h"
#include "vm/vm.h"
#include <stdbool.h>

#define MAX_OVERRIDE_ENTRIES 64

typedef struct {
    char target_name[64];
    long target_line;
    char replacement_sub[64];
    bool is_active;
} BppOverrideEntry;

typedef struct {
    BppOverrideEntry entries[MAX_OVERRIDE_ENTRIES];
    int count;
} BppOverrideTable;

void override_init(VMContext *vm);
void override_clear(VMContext *vm);
bool override_register(VMContext *vm, const char *target_name, long target_line, const char *replacement_sub);
const BppOverrideEntry *override_lookup(VMContext *vm, const char *target_name);

#endif // RUNTIME_OVERRIDE_H
