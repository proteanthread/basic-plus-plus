// FILENAME: math.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libext, libkernel, libplatform
// NEEDS: libengine (vm.h, vm_internal.h)
// Implements bytecode virtual machine execution and state for math.
//
// ---- Includes ----

#include "vm/vm.h"
#include "vm_internal.h"

double vm_get_last_rnd(VMContext *vm) { return vm ? vm->last_rnd : 0.0; }
void vm_set_last_rnd(VMContext *vm, double val) { if (vm) vm->last_rnd = val; }
