#ifndef BASICPP_TASK_H
#define BASICPP_TASK_H

#include "runtime.h"

#if !defined(_WIN32) && (defined(__linux__) || defined(__APPLE__) || defined(__unix__))
#include <pthread.h>
#endif

#define MAX_TASKS 32

typedef enum TaskState {
    TASK_RUNNING_FG, // Foreground running (Green)
    TASK_WAITING,    // Waiting on input (Yellow)
    TASK_RUNNING_BG, // Background running (White)
    TASK_ERROR,      // Done with error (Red)
    TASK_DONE,       // Clean exit (Blue)
    TASK_DONE_ERR    // Exit with non-clean code (Purple)
} TaskState;

typedef struct BasicTask {
    int pid;
    char filename[260];
    RuntimeState *rt;
    MemorySystem *mem;
    TaskState state;
    int active_bank_id;
    int is_used;
    int is_threaded;
#if defined(_WIN32)
    void *thread_handle;
    unsigned long thread_id;
#elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
    pthread_t thread_handle;
    int has_thread;
#endif
} BasicTask;

// Mutex API for thread-safe shared RAMBANK access
void task_mutex_lock(void);
void task_mutex_unlock(void);

// Task Manager API
void task_mgr_init(RuntimeState *main_rt);
void task_mgr_shutdown(void);
int task_spawn(const char *filename, int line_num);
void task_list(void);
void task_switch(int target_pid, int line_num);
void task_scheduler_tick(void);
BasicTask *task_get_current(void);
int task_has_background_active(void);
void task_mark_waiting(int waiting);
MemorySystem *task_get_main_mem(void);
void task_kill(int pid, int line_num);
void task_join(int pid, RuntimeState *rt, int line_num);
int task_get_status(int pid);

#endif // BASICPP_TASK_H
