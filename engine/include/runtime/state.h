// FILENAME: state.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (state.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for state within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_STATE_H
#define RUNTIME_STATE_H

#include "types/types.h"
#include "vm/vm.h"

BppError vm_state_save(VMContext *vm, const char *filename);
BppError vm_state_load(VMContext *vm, const char *filename);
char *vm_state_info(VMContext *vm, const char *filename, BppError *err);

#endif // RUNTIME_STATE_H
