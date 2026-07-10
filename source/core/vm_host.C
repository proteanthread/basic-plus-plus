#include "vm_host.H"
#include "device_alias.h"
#include <string.h>
#include <stdio.h>

#include "task.H"

static VirtualMachine vm_instances[MAX_VM_INSTANCES];
static int vm_host_initialized = 0;

void vm_host_init(void) {
    if (vm_host_initialized) return;
    memset(vm_instances, 0, sizeof(vm_instances));
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        vm_instances[i].vm_id = -1;
        vm_instances[i].state = VM_STATE_STOPPED;
        vm_instances[i].task_pid = -1;
    }
    vm_host_initialized = 1;
}

void vm_host_shutdown(void) {
    if (!vm_host_initialized) return;
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        if (vm_instances[i].vm_id != -1) {
            vm_host_stop(vm_instances[i].vm_id);
        }
    }
    vm_host_initialized = 0;
}

int vm_host_create(const char *name, MockBiosModel model, int ram_kb) {
    if (!vm_host_initialized) vm_host_init();
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        if (vm_instances[i].vm_id == -1) {
            slot = i;
            break;
        }
    }
    if (slot == -1) return -1; // Max VMs reached
    
    VirtualMachine *vm = &vm_instances[slot];
    vm->vm_id = slot;
    strncpy(vm->name, name, sizeof(vm->name) - 1);
    vm->name[sizeof(vm->name)-1] = '\0';
    vm->model = model;
    vm->state = VM_STATE_STOPPED;
    vm->task_pid = -1;
    vm->is_attached = 0;
    vm->target_speed_pct = 100;
    vm->interrupt_head = 0;
    vm->interrupt_tail = 0;
    
    // Initialize Segmented Memory (allocate isolated RAMBANK)
    // For now, let's just initialize the struct. Actual RAMBANK allocation happens when started, 
    // or we can manually trigger it here if segmented_mem_init is updated to take instances.
    vm->mem_ctx = gw_mem_create(ram_kb * 1024);
    
    // Initialize Mock BIOS
    mock_bios_init_mem(&vm->bios_ctx, gw_mem_get_buffer(vm->mem_ctx), gw_mem_get_size(vm->mem_ctx), model);
    device_alias_load_machine(model);
    
    return slot;
}

int vm_host_start(int vm_id, const char *boot_file) {
    if (vm_id < 0 || vm_id >= MAX_VM_INSTANCES) return -1;
    VirtualMachine *vm = &vm_instances[vm_id];
    if (vm->vm_id == -1) return -1;
    
    if (vm->state != VM_STATE_STOPPED) return -1; // Already running
    
    vm->state = VM_STATE_STARTING;
    
    // Spawn task
    int pid = task_spawn(boot_file, 0); // Spawns a background task for this VM
    if (pid < 0) {
        vm->state = VM_STATE_ERROR;
        return -1;
    }
    
    vm->task_pid = pid;
    vm->state = VM_STATE_RUNNING;
    return 0;
}

int vm_host_stop(int vm_id) {
    if (vm_id < 0 || vm_id >= MAX_VM_INSTANCES) return -1;
    VirtualMachine *vm = &vm_instances[vm_id];
    if (vm->vm_id == -1) return -1;
    
    if (vm->task_pid != -1) {
        task_kill(vm->task_pid, 0);
        vm->task_pid = -1;
    }
    
    vm->state = VM_STATE_STOPPED;
    return 0;
}

int vm_host_attach(int vm_id) {
    if (vm_id < 0 || vm_id >= MAX_VM_INSTANCES) return -1;
    VirtualMachine *vm = &vm_instances[vm_id];
    if (vm->vm_id == -1) return -1;
    
    // Detach all others
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        vm_instances[i].is_attached = 0;
    }
    
    // Attach this one
    vm->is_attached = 1;
    return 0;
}

int vm_host_find_by_name(const char *name) {
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        if (vm_instances[i].vm_id != -1 && strcmp(vm_instances[i].name, name) == 0) {
            return vm_instances[i].vm_id;
        }
    }
    return -1;
}

VirtualMachine* vm_host_get(int vm_id) {
    if (vm_id < 0 || vm_id >= MAX_VM_INSTANCES) return NULL;
    if (vm_instances[vm_id].vm_id == -1) return NULL;
    return &vm_instances[vm_id];
}

VirtualMachine* vm_host_current(void) {
    // Find VM based on the current task
    BasicTask *cur_task = task_get_current();
    if (!cur_task) return NULL;
    
    for (int i = 0; i < MAX_VM_INSTANCES; i++) {
        if (vm_instances[i].vm_id != -1 && vm_instances[i].task_pid == cur_task->pid) {
            return &vm_instances[i];
        }
    }
    return NULL;
}

void vm_host_set_speed(int vm_id, int pct) {
    VirtualMachine *vm = vm_host_get(vm_id);
    if (vm) {
        if (pct < 1) pct = 1;
        if (pct > 100) pct = 100;
        vm->target_speed_pct = pct;
    }
}

void vm_host_inject_interrupt(int vm_id, uint8_t int_num) {
    VirtualMachine *vm = vm_host_get(vm_id);
    if (vm) {
        int next_tail = (vm->interrupt_tail + 1) % 16;
        if (next_tail != vm->interrupt_head) { // If queue not full
            vm->interrupt_queue[vm->interrupt_tail] = int_num;
            vm->interrupt_tail = next_tail;
        }
    }
}
