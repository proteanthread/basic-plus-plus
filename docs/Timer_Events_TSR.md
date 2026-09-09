<!--
Title:        Timer_Events_TSR
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/timer.c, engine/src/runtime/timer.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Timer Events & TSR Architecture

The authoritative specification for asynchronous timer event traps, interval handlers, and cooperative TSR-style background execution in BASIC++ v6.5.2.

---

## 1. Event Trapping Model

BASIC++ provides asynchronous timer event trapping modeled on vintage Microsoft BASIC and enhanced for modern real-time execution. Timer events execute cooperatively at statement dispatch boundaries:
- Event handlers are invoked without interrupting long scalar math operations mid-instruction.
- When an event triggers, the current program position is saved and execution jumps to the registered trap line or procedure.
- Execution returns to the point of interruption upon encountering `RETURN` or `END EVENT`.

---

## 2. Timer Trapping Statements

- **`ON TIMER(interval) GOSUB line_num|label`**: Registers a periodic timer event trap that triggers every `interval` seconds (supports floating-point fractional seconds, e.g., `0.05` for 20 Hz).
- **`TIMER ON`**: Enables active polling and triggering of the registered timer event trap.
- **`TIMER OFF`**: Disables the timer event trap and clears any queued pending timer events.
- **`TIMER STOP`**: Suspends timer event invocation while retaining queued events; when `TIMER ON` is subsequently executed, any pending event triggers immediately.

---

## 3. TSR-Style Background Execution

In embedded microcontrollers (`iot`) and desktop consoles (`baspp`), timer event trapping allows writing non-blocking "Terminate and Stay Resident" (TSR) style utility loops:
- Background telemetry loggers, watchdog heartbeats, and sensor polling tasks operate via `ON TIMER` while the main script executes foreground calculations or user input.
- `SLEEP interval` or `YIELD` allows the VM to sleep in low-power mode while maintaining precise timer responsiveness.

---

## 4. Example: Periodic Heartbeat Event Trap

```basic
10 REM Timer Event Trap Demo
20 Ticks = 0
30 ON TIMER(1) GOSUB 1000
40 TIMER ON
50 PRINT "Timer trap enabled. Main program running..."
60 FOR I = 1 TO 5
70   PRINT "Working in foreground loop: "; I
80   SLEEP 1.5
90 NEXT I
100 TIMER OFF
110 PRINT "Timer disabled. Final tick count: "; Ticks
120 END
1000 REM Timer Event Handler
1010 Ticks = Ticks + 1
1020 PRINT "  [HEARTBEAT] Tick event #"; Ticks; " at "; TIME$
1030 RETURN
```
