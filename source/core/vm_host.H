#ifndef BASICPP_VM_HOST_H
#define BASICPP_VM_HOST_H

#include <stdint.h>
#include "standalone/mock_bios/mock_bios_core.H"
#include "segmented_mem.H"
#include "vdev.H"

#define MAX_VM_INSTANCES 16

// VM Execution States
typedef enum HostVMState {
    VM_STATE_STOPPED,
    VM_STATE_STARTING,
    VM_STATE_RUNNING,
    VM_STATE_PAUSED,
    VM_STATE_ERROR
} HostVMState;

// Virtual Machine Context
// Encapsulates all independent state for a single simulated machine.
typedef struct VirtualMachine {
    int vm_id;                          // Unique ID for the VM (matches task ID or slot)
    char name[32];                      // User-defined name for the VM
    HostVMState state;                      // Current lifecycle state
    MockBiosModel model;                // The hardware model (e.g. XT, AT, PCjr)
    
    // Core isolated sub-systems
    GW_Memory *mem_ctx;        // Isolated 1MB segment and A20 gate
    MockBiosContext bios_ctx;           // Isolated BIOS/DOS state (BDA, IVT, Handles)
    // VDevRegistry vdev_registry;      // (TODO: define VDevRegistry in vdev.H to encapsulate devices)
    
    // Multitasking
    int task_pid;                       // PID of the BasicTask executing this VM
    int is_attached;                    // If 1, this VM is drawing to the main SDL window
    
    // Cycle Throttling
    int target_speed_pct;               // 100 = full host speed, <100 = throttled
    
    // Simulated 8259 PIC (Interrupt Queue)
    uint8_t interrupt_queue[16];
    int interrupt_head;
    int interrupt_tail;
} VirtualMachine;

// Initialize the global VM manager
void vm_host_init(void);

// Shut down all VMs
void vm_host_shutdown(void);

// Create a new Virtual Machine instance
// returns VM ID on success, -1 on error.
int vm_host_create(const char *name, MockBiosModel model, int ram_kb);

// Start executing a VM (maps to task_spawn internally)
int vm_host_start(int vm_id, const char *boot_file);

// Stop a VM
int vm_host_stop(int vm_id);

// Attach VM graphical/console output to the foreground
int vm_host_attach(int vm_id);

// Throttle VM execution speed
void vm_host_set_speed(int vm_id, int pct);

// Inject hardware interrupt into VM PIC queue
void vm_host_inject_interrupt(int vm_id, uint8_t int_num);

// Find VM by name
int vm_host_find_by_name(const char *name);

// Get VM by ID
VirtualMachine* vm_host_get(int vm_id);

// Get the currently executing VM (based on current task)
VirtualMachine* vm_host_current(void);

#endif // BASICPP_VM_HOST_H
