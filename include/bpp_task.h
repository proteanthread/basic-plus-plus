/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_task.h
 * @brief Multitasking Task Manager API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures and APIs for managing concurrent background tasks.
 * - Why it exists: Provides process isolation and multitasking executing multiple BASIC files concurrently.
 * - Why it works this way: It allocates independent VM contexts for spawned processes and manages
 *   thread worker lifetimes on native OS thread systems.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Maximum task counts, thread worker loop delay parameters.
 * - What cannot be changed: Main task PID (always 0), scheduler tick interfaces.
 * - What to expect: Spawning creates background threads. Switching changes foreground context.
 * - What to do if something breaks: Trace thread states, inspect process exit codes.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Relies on Windows threads or POSIX threads.
 * - Portability concerns: Stubbed on FreeDOS. Standard C17.
 */

#ifndef BPP_TASK_H
#define BPP_TASK_H

#include "bpp_vdev.h"
#include "bpp_platform.h"

#define MAX_TASKS 32

typedef enum {
    TASK_RUNNING_FG, /* Foreground running (Green) */
    TASK_WAITING,    /* Waiting on input (Yellow) */
    TASK_RUNNING_BG, /* Background running (White) */
    TASK_ERROR,      /* Done with error (Red) */
    TASK_DONE,       /* Clean exit (Blue) */
    TASK_DONE_ERR    /* Exit with non-clean code (Purple) */
} BppTaskState;

typedef struct {
    int          pid;
    char         filename[260];
    void        *vm;             /* Pointer to private VMContext */
    void        *mem;            /* Pointer to private MemoryContext */
    BppTaskState state;
    int          is_used;
    int          is_threaded;
    BppThread    thread;
} BppBasicTask;

void task_mutex_lock(void);
void task_mutex_unlock(void);

void task_mgr_init(void *main_vm);
void task_mgr_shutdown(void);
int  task_mgr_has_active_tasks(void);
int  task_spawn(VDevContext *vdev, const char *filename);
int  task_spawn_at_label(VDevContext *vdev, const char *filename, const char *label);
void task_list(VDevContext *vdev);
void task_switch(VDevContext *vdev, int target_pid);
void task_scheduler_tick(void);
void task_kill(VDevContext *vdev, int pid);
void task_join(int pid);
int  task_get_status(int pid);

#endif /* BPP_TASK_H */
