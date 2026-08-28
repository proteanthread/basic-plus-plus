# C17 API Reference: Multitasking & Worker Subsystem (`runtime/task.h`)

## 1. Subsystem Overview & Responsibilities

The Multitasking & Worker Subsystem (`runtime/task.h`, implemented in `engine/src/runtime/task.c`) manages cooperative and preemptive background worker tasks (`TASK`), OS thread orchestration, process isolation, and task synchronization for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Task Table Management**: Tracks up to 32 active concurrent worker tasks (`MAX_TASKS`).
- **Process State Lifecycle (`BppTaskState`)**:
  - `TASK_RUNNING_FG`: Running in foreground.
  - `TASK_WAITING`: Blocked waiting on I/O or mutex lock.
  - `TASK_RUNNING_BG`: Running asynchronously in background worker thread.
  - `TASK_DONE` / `TASK_ERROR`: Finished execution with exit status code.
- **Isolated VM Execution Frames**: Each spawned task executes within its own private `VMContext` and `MemoryContext`, preventing memory race conditions on variable tables.
- **Thread Synchronization**: Provides thread joins (`task_join()`), termination (`task_kill()`), and global interpreter mutex locks (`task_mutex_lock()`, `task_mutex_unlock()`).

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/task.h"
#include "device/vdev.h"
#include "platform/platform.h"
```

## 3. Data Structures & Types

```c
#define MAX_TASKS 32

typedef enum {
    TASK_RUNNING_FG, /* Foreground running */
    TASK_WAITING,    /* Waiting on input/event */
    TASK_RUNNING_BG, /* Background running */
    TASK_ERROR,      /* Terminated with error */
    TASK_DONE,       /* Clean termination */
    TASK_DONE_ERR    /* Exit with non-zero error code */
} BppTaskState;

/* Task Descriptor Struct */
typedef struct {
    int          pid;           /* Unique task process identifier (1..32) */
    char         filename[260]; /* Script filename */
    void        *vm;            /* Private VMContext pointer */
    void        *mem;           /* Private MemoryContext pointer */
    BppTaskState state;         /* Current task state */
    int          is_used;       /* Slot in-use flag */
    int          is_threaded;   /* 1 if running on native OS thread */
    BppThread    thread;        /* OS thread handle */
} BppBasicTask;
```

## 4. Function Prototypes & Operational Contracts

### Manager Lifecycle & Spawning
```c
/**
 * @brief Initializes the multitasking manager.
 */
void task_mgr_init(void *main_vm);

/**
 * @brief Shuts down the task manager, terminating all active background workers.
 */
void task_mgr_shutdown(void);

/**
 * @brief Spawns an external BASIC script as a background worker task.
 * @return Assigned PID (1..32), or negative error code on failure.
 */
int task_spawn(VDevContext *vdev, const char *filename);

/**
 * @brief Spawns a background task starting at a specific label in the current file.
 */
int task_spawn_at_label(VDevContext *vdev, const char *filename, const char *label);
```

### Task Control & Synchronization
```c
void task_list(VDevContext *vdev);
void task_switch(VDevContext *vdev, int target_pid);
void task_kill(VDevContext *vdev, int pid);
void task_join(int pid);
int  task_get_status(int pid);

void task_mutex_lock(void);
void task_mutex_unlock(void);
```

## 5. Architectural Invariants

- **Memory Isolation**: Background tasks execute in isolated memory heaps to avoid data races.
- **Clean Worker Joins**: The main process waits for background tasks during `boot_shutdown()`.

## 6. Code Example: Spawning a Background Worker in BASIC++

```basic
10 PRINT "Launching background cruncher task..."
20 LET WORKER_PID = TASK SPAWN "COMPUTE_WORKER.BAS"
30 PRINT "Worker running with PID: "; WORKER_PID
40 REM Main thread continues execution...
50 TASK JOIN WORKER_PID
60 PRINT "Worker completed."
```
