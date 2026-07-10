#ifndef BASICPP_VM_SNAPSHOT_H
#define BASICPP_VM_SNAPSHOT_H

#include "../core/vm_host.H"

// Save VM state to a snapshot file
// format: 0 = VMS binary, 1 = Raw + JSON
int vm_snapshot_save(VirtualMachine* vm, const char* filename, int format);

// Load VM state from a snapshot file
int vm_snapshot_load(VirtualMachine* vm, const char* filename);

#endif
