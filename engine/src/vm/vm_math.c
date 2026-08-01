/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "vm/vm.h"
#include "vm_internal.h"

double vm_get_last_rnd(VMContext *vm) { return vm ? vm->last_rnd : 0.0; }
void vm_set_last_rnd(VMContext *vm, double val) { if (vm) vm->last_rnd = val; }
