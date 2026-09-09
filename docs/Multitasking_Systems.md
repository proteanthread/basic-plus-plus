<!--
Title:        Multitasking_Systems
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/task.c, engine/src/runtime/task.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Multitasking Systems Architecture

The authoritative specification for cooperative multitasking, concurrent script task execution, task life cycles, and execution scheduling in BASIC++ v6.5.2.

---

## 1. Cooperative Multitasking Model

BASIC++ implements cooperative multitasking within a deterministic single-threaded virtual machine context. Concurrent tasks execute interleaved at statement boundaries: each scheduled task executes statements within its time slice and cooperatively yields control back to the task scheduler.

Because execution is cooperative rather than preemptive:
- Long computations must periodically call `YIELD` to give processing time to other queued tasks.
- Hardware interrupts, timer events, and network packets are polled and serviced at statement dispatch boundaries.
- Multitasking operations require `SECOP_SYSTEM` security privilege; unprivileged sandboxes restrict background spawning.

The multitasking subsystem is implemented in `engine/src/statements/system/task.c` and `engine/src/runtime/task.c`.

---

## 2. Task Management Statements

### A. The `TASK` Statement
The `TASK` statement manages task creation, inspection, synchronization, and termination:

- **`TASK` or `TASK LIST`**: Lists all active background tasks, their task identifiers (PIDs), and their current execution states.
- **`TASK "filename.bas"`**: Spawns an independent background task executing the specified BASIC script file.
- **`TASK ::global_label`**: Spawns a background task beginning execution at the designated global label within the currently loaded program.
- **`TASK pid`**: Switches active execution focus to the specified task ID.
- **`TASK WAIT pid`**: Blocks the calling routine until the background task identified by `pid` terminates (task join).
- **`TASK KILL pid`**: Immediately terminates execution of the background task identified by `pid` and reclaims its resources.

### B. Cooperative Yielding and Synchronization
- **`YIELD`**: Voluntarily relinquishes the current VM execution slice to the next ready task in the scheduling ring.
- **`SUSPEND`**: Pauses execution of the current or specified task until an explicit resume signal is received.
- **`RESUME`**: Resumes execution of a previously suspended task or re-enables event trapping after handling.

---

## 3. Example: Background Worker Task

```basic
10 REM Main Task Controller
20 PRINT "Starting background logger task..."
30 TASK "logger.bas"
40 FOR I = 1 TO 10
50   PRINT "Main loop iteration: "; I
60   YIELD
70 NEXT I
80 TASK LIST
90 END
```
