# `task` Background Task & Scheduler Subsystem (`libserver`)

## 1. Architectural Purpose & Overview

The `task` subsystem (`engine/src/server/task.c`) manages background asynchronous worker threads, recurring timer tasks, and event-driven multitasking.

### Key Architectural Invariants:
- **Thread Safety**: Task state transitions use mutex-protected event rings.
- **VM State Isolation**: Background tasks execute within isolated sub-contexts and communicate via message queues.

---

## 2. Technical API Signatures (C17)

```c
TaskHandle task_spawn(TaskCallback callback, void *user_data);
void task_wait(TaskHandle handle);
void task_cancel(TaskHandle handle);
```
