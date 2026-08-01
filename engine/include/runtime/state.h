/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_state.h
 * @brief Session State Save/Load Interface.
 */

#ifndef BPP_STATE_H
#define BPP_STATE_H

#include "types/types.h"
#include "vm/vm.h"

BppError vm_state_save(VMContext *vm, const char *filename);
BppError vm_state_load(VMContext *vm, const char *filename);
char *vm_state_info(VMContext *vm, const char *filename, BppError *err);

#endif /* BPP_STATE_H */
