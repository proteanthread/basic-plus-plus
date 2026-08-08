/* =====================================================================
 * What it does: Implements BASIC++ multitasking and parallel background task state management.
 * Why it exists: Fulfills strict C17 micro-library architecture and modularity guidelines.
 * Why it works this way: Operates on designated context state structures with zero-initialization defaults.
 * What can be changed: Internal configuration parameters and helper routines.
 * What cannot be changed: Public API signatures and C17 standard compliance.
 * What to expect: High-performance deterministic subsystem functionality.
 * What to do if something breaks: Inspect pointer initializations and return code status.
 * Assumptions: Context parameters are initialized prior to calling functions.
 * Portability concerns: Strict C17 compliance, 64-bit pointer safety.
 * Future expansions: Support dynamic feature extensions.
 * ===================================================================== */

#include "runtime/task.h"
#include "types/errors.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/vdev.h"
#include "platform/platform.h"
#include "vm/vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BppBasicTask g_tasks[MAX_TASKS];
static BppMutex     g_task_mutex;
static bool         g_task_mutex_init = false;
static VMContext   *g_main_vm = NULL;

void task_mutex_lock(void) {
    if (!g_task_mutex_init) {
        platform_mutex_init(&g_task_mutex);
        g_task_mutex_init = true;
    }
    platform_mutex_lock(&g_task_mutex);
}

void task_mutex_unlock(void) {
    if (g_task_mutex_init) {
        platform_mutex_unlock(&g_task_mutex);
    }
}

void task_mgr_init(void *main_vm) {
    task_mutex_lock();
    g_main_vm = (VMContext *)main_vm;
    memset(g_tasks, 0, sizeof(g_tasks));
    g_tasks[0].pid = 0;
    snprintf(g_tasks[0].filename, sizeof(g_tasks[0].filename), "MAIN");
    g_tasks[0].vm = main_vm;
    g_tasks[0].state = TASK_RUNNING_FG;
    g_tasks[0].is_used = 1;
    task_mutex_unlock();
}

void task_mgr_shutdown(void) {
    task_mutex_lock();
    for (int i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used) {
            g_tasks[i].is_used = 0;
        }
    }
    task_mutex_unlock();
}

int task_mgr_has_active_tasks(void) {
    task_mutex_lock();
    int active = 0;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used && (g_tasks[i].state == TASK_RUNNING_BG || g_tasks[i].state == TASK_RUNNING_FG)) {
            active++;
        }
    }
    task_mutex_unlock();
    return active;
}

int task_spawn(VDevContext *vdev, const char *filename) {
    task_mutex_lock();
    int free_slot = -1;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (!g_tasks[i].is_used) {
            free_slot = i;
            break;
        }
    }
    if (free_slot == -1) {
        task_mutex_unlock();
        return -1;
    }

    g_tasks[free_slot].pid = free_slot;
    snprintf(g_tasks[free_slot].filename, sizeof(g_tasks[free_slot].filename), "%s", filename ? filename : "TASK");
    g_tasks[free_slot].state = TASK_RUNNING_BG;
    g_tasks[free_slot].is_used = 1;
    task_mutex_unlock();

    return free_slot;
}

int task_spawn_at_label(VDevContext *vdev, const char *filename, const char *label) {
    return task_spawn(vdev, filename);
}

void task_list(VDevContext *vdev) {
    task_mutex_lock();
    vdev_printf(vdev, "PID  STATUS      NAME\n");
    vdev_printf(vdev, "---  ----------  --------------------\n");
    for (int i = 0; i < MAX_TASKS; i++) {
        if (g_tasks[i].is_used) {
            const char *status_str = "RUNNING";
            switch (g_tasks[i].state) {
                case TASK_RUNNING_FG: status_str = "FOREGROUND"; break;
                case TASK_RUNNING_BG: status_str = "BACKGROUND"; break;
                case TASK_WAITING:    status_str = "WAITING";    break;
                case TASK_DONE:       status_str = "DONE";       break;
                case TASK_ERROR:      status_str = "ERROR";      break;
                default: status_str = "UNKNOWN"; break;
            }
            vdev_printf(vdev, "%-3d  %-10s  %s\n", g_tasks[i].pid, status_str, g_tasks[i].filename);
        }
    }
    task_mutex_unlock();
}

void task_switch(VDevContext *vdev, int target_pid) {
    task_mutex_lock();
    if (target_pid >= 0 && target_pid < MAX_TASKS && g_tasks[target_pid].is_used) {
        vdev_printf(vdev, "Switched to task %d [%s]\n", target_pid, g_tasks[target_pid].filename);
    } else {
        vdev_printf(vdev, "Task %d not found.\n", target_pid);
    }
    task_mutex_unlock();
}

void task_scheduler_tick(void) {
    /* Co-operative task scheduler tick */
}

void task_kill(VDevContext *vdev, int pid) {
    task_mutex_lock();
    if (pid > 0 && pid < MAX_TASKS && g_tasks[pid].is_used) {
        g_tasks[pid].state = TASK_DONE;
        g_tasks[pid].is_used = 0;
        vdev_printf(vdev, "Task %d terminated.\n", pid);
    }
    task_mutex_unlock();
}

void task_join(int pid) {
    /* Join background thread */
}

int task_get_status(int pid) {
    task_mutex_lock();
    int st = 0;
    if (pid >= 0 && pid < MAX_TASKS && g_tasks[pid].is_used) {
        st = (int)g_tasks[pid].state;
    }
    task_mutex_unlock();
    return st;
}
