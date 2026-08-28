// FILENAME: exec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (exec_internal.h, vm.h, vm_internal.h)
// Implements bytecode virtual machine execution and state for exec.
//
// ---- Includes ----

#include "vm_internal.h"
#include "vm/exec_internal.h"
#include "vm/vm.h"

//
// ---- Exec Subsystem Entry Point ----

// initializes and registers all statement handlers for the VM instance
void vm_exec_init(VMContext *vm) {
    if (!vm) return;
    register_core_statements(vm);
}
