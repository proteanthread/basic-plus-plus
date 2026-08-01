# Multitasking Subsystem API Reference

Header File: [`include/bpp_task.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_task.h)

## Overview
Controls cooperative thread execution, task spawn, wait, yield, and priority channels.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `task_mutex_lock` | `void` | `void` |
| `task_mutex_unlock` | `void` | `void` |
| `task_mgr_init` | `void` | `void *main_vm` |
| `task_mgr_shutdown` | `void` | `void` |
| `task_spawn` | `int` | `VDevContext *vdev, const char *filename` |
| `task_spawn_at_label` | `int` | `VDevContext *vdev, const char *filename, const char *label` |
| `task_list` | `void` | `VDevContext *vdev` |
| `task_switch` | `void` | `VDevContext *vdev, int target_pid` |
| `task_scheduler_tick` | `void` | `void` |
| `task_kill` | `void` | `VDevContext *vdev, int pid` |
| `task_join` | `void` | `int pid` |
| `task_get_status` | `int` | `int pid` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_task.h"

void yield_task(VMContext *vm) {
    task_yield(vm);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
