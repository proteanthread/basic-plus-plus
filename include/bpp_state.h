/**
 * @file bpp_state.h
 * @brief Session State Save/Load Interface.
 */

#ifndef BPP_STATE_H
#define BPP_STATE_H

#include "bpp_types.h"
#include "bpp_vm.h"

BppError vm_state_save(VMContext *vm, const char *filename);
BppError vm_state_load(VMContext *vm, const char *filename);
char *vm_state_info(VMContext *vm, const char *filename, BppError *err);

#endif /* BPP_STATE_H */
