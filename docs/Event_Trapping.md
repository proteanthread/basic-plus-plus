# BASIC++ Event & Error Trapping Tutorial


---

## Table of Contents

- Overview
- Error Trapping
  - 1 ON ERROR GOTO (GW-BASIC / QBasic Style)
  - 2 TRAP n (Atari BASIC Style)
  - 3 Bare TRAP (Status Query)
- Structured Exception Handling (ECMA-116)
- Event Trapping
  - 1 ON event GOSUB/GOTO
  - 2 Event Enable/Disable
  - 3 How Event Polling Works
  - 4 ON BREAK — OS Signal Handler
  - 5 Device I/O Events (VDev Tier 2)
  - 6 UPnP / Network Events (Tier 5)
- ON TRAP — The Hybrid Event Router
  - 1 Syntax
  - 2 Event Classes
  - 3 Examples
  - 4 Dialect Conflicts
- The TRAP Three-Argument Form
- Error Codes (ERR)
- Comparison of Trapping Styles
- Best Practices
- Quick Reference

---

## Overview

BASIC++ provides a unified trapping system for handling errors, events, and
interrupts.  Four families of syntax are supported, drawn from the dialects
where each style originated, plus BASIC++ extensions for OS signals, device
I/O, and network events:

| Syntax Family          | Origin          | Example                         |
|------------------------|-----------------|----------------------------------|
| `TRAP n`               | Atari BASIC     | `TRAP 1000`                      |
| `ON ERROR GOTO n`      | GW-BASIC        | `ON ERROR GOTO 500`              |
| `ON TRAP GOTO / GOSUB` | BASIC++ hybrid  | `ON TRAP(1) GOSUB 2000`         |
| `WHEN EXCEPTION IN`    | ECMA-116        | structured try/catch blocks      |
| `ON BREAK GOSUB n`     | BASIC++         | intercept Ctrl+C / OS signals    |

Because `TRAP` means different things in different dialects, BASIC++ treats
it as a **hybrid command**: in Atari mode it behaves like Atari BASIC's
`TRAP n`; in other modes it serves as a general-purpose event router.

---

## 1. Error Trapping

### 1.1 ON ERROR GOTO (GW-BASIC / QBasic Style)

The most widely supported form.  When a runtime error occurs, execution
transfers to the handler instead of halting.

```basic
10 ON ERROR GOTO 100
20 PRINT 1/0            : REM division by zero
30 PRINT "This line never runs"
40 END

100 REM --- Error Handler ---
110 PRINT "Error"; ERR; "at line"; ERL
120 RESUME NEXT
```

**Key rules:**
- `ON ERROR GOTO 0` disables the handler.
- Inside the handler, `ERR` holds the error code and `ERL` the line
  where the error occurred.
- `RESUME` retries the failed line.
- `RESUME NEXT` continues at the line after the failure.
- `RESUME n` jumps to line n.

### 1.2 TRAP n (Atari BASIC Style)

Atari BASIC uses `TRAP` as a compact alternative to `ON ERROR GOTO`.

```basic
10 TRAP 100
20 PRINT 1/0
30 END

100 REM --- Trap Handler ---
110 PRINT "Caught error at line"; ERL
120 REM In Atari BASIC, the trap auto-disables after firing.
130 REM In BASIC++, it stays active until TRAP 0.
```

**Equivalence:**
```
TRAP 100          is equivalent to     ON ERROR GOTO 100
TRAP 0            is equivalent to     ON ERROR GOTO 0
```

**Hybrid note:** When the active dialect is **Atari BASIC** (`-d ATAR`),
`TRAP` is identical to Atari's original behavior.  In all other
dialects, `TRAP` additionally supports the extended three-argument form
and the `ON TRAP` event syntax described below.

### 1.3 Bare TRAP (Status Query)

Typing `TRAP` with no arguments prints the current trap status:

```
TRAP
TRAP active -> line 100

TRAP 0
TRAP
TRAP inactive
```

---

## 2. Structured Exception Handling (ECMA-116)

For more complex error recovery, BASIC++ supports ECMA-116 structured
exception blocks.

```basic
100 WHEN EXCEPTION IN
110   OPEN "MISSING.DAT" FOR INPUT AS #1
120   PRINT "File opened OK"
130   CLOSE #1
140 USE
150   PRINT "Cannot open file (ERR ="; ERR; ")"
160 END WHEN
170 PRINT "Program continues here"
```

**Keywords:**
| Keyword    | Purpose                                        |
|------------|------------------------------------------------|
| `WHEN EXCEPTION IN` | Start a protected block              |
| `USE`      | Start the exception handler                    |
| `END WHEN` | End the entire block                           |
| `RETRY`    | Re-enter the protected block from the handler  |
| `CONTINUE` | Skip the failed statement and continue         |

**Nesting:** WHEN blocks can be nested.  Each block has its own handler.

```basic
100 WHEN EXCEPTION IN
110   WHEN EXCEPTION IN
120     PRINT 1/0
130   USE
140     PRINT "Inner handler caught it"
150   END WHEN
160 USE
170   PRINT "Outer handler (should not fire)"
180 END WHEN
```

---

## 3. Event Trapping

BASIC++ supports GW-BASIC/QBasic-style event trapping with `ON event GOSUB`.
These register a subroutine to call when a specific event fires.

### 3.1 ON event GOSUB/GOTO

```basic
10 ON TIMER(5) GOSUB 500        : REM every 5 seconds
20 ON KEY(1) GOSUB 600           : REM F1 key
30 ON COM(1) GOSUB 700           : REM serial port 1
40 ON STRIG(0) GOSUB 800         : REM joystick button
50 ON PEN GOSUB 900              : REM light pen
60 ON TRAP GOSUB 1000            : REM general trap event
```

### 3.2 Event Enable/Disable

Events are controlled with three-state switches: **ON**, **OFF**, **STOP**.

```basic
TIMER ON              : REM enable TIMER event checking
TIMER OFF             : REM disable TIMER event checking
TIMER STOP            : REM suspend (events queue, fire on ON)
```

Same pattern for `COM ON/OFF/STOP`, `KEY ON/OFF/STOP`,
`PEN ON/OFF/STOP`, `STRIG ON/OFF/STOP`.

### 3.3 How Event Polling Works

BASIC++ checks for pending events **between statements**.  When an
event fires:

1. The current line finishes its current statement.
2. A `GOSUB` is issued to the registered handler.
3. The handler runs and ends with `RETURN`.
4. Execution resumes where it left off.

This is **cooperative**, not preemptive.  A tight loop like
`10 GOTO 10` will check events between iterations because GOTO
returns to the statement dispatcher.

### 3.4 ON BREAK — OS Signal Handler

Intercept Ctrl+C (SIGINT) and other OS termination signals.

```basic
10 ON BREAK GOSUB 500
20 PRINT "Press Ctrl+C to test..."
30 FOR I = 1 TO 100000
40   REM busy work
50 NEXT I
60 END

500 PRINT ">> Ctrl+C intercepted!"
510 PRINT "Cleaning up..."
520 RETURN
```

**Rules:**
- `ON BREAK GOTO 0` disables the handler (restores default `[BREAK]` behavior).
- If no handler is registered, Ctrl+C stops the program normally.
- **Warning:** If the handler never ends the program (`END` or `STOP`), it
  becomes unkillable via keyboard.  The `STOP` statement and debugger `BREAK`
  command always work regardless.

### 3.4b ON ALARM / ON ALARM$ — Countdown & Daily Alarms

BASIC++ supports countdown and daily repeating alarms with `ON ALARM` and `ON ALARM$`. Alarms are configured and controlled using the `SET` keyword and modifiers: `ON`, `OFF`, `STOP`, `PAUSE`, `SNOOZE [m]` (defaults to 300 seconds), and `UNSET` (removes the alarm definition).

*   **Countdown Alarms (`ALARM(n)`):**
    Allows setting a timer that counts down `n` seconds (supports fractional values, 1 to 86400).
    ```basic
    10 ON ALARM(1.5) GOSUB 100        : REM trap countdown of 1.5 seconds
    20 SET ALARM(1.5) ON              : REM start/activate the alarm
    30 ALARM ON                       : REM enable global alarms check
    40 PRINT ALARM(1.5)               : REM prints remaining seconds
    50 GOTO 40
    100 PRINT "Alarm fired!" : RETURN
    ```
    Modifiers:
    - `SET ALARM(n) ON`: Start or resume the timer.
    - `SET ALARM(n) OFF` / `STOP` / `PAUSE`: Stop or pause the timer.
    - `SET ALARM(n) SNOOZE [secs]`: Snooze the timer by adding `secs` to it (defaults to 300).
    - `SET ALARM(n) UNSET`: Completely delete and clear the alarm.
    - `ALARM ON / OFF / STOP` controls the global countdown alarms state.

*   **Daily Alarms (`ALARM$(time)`):**
    Daily repeating alarms execute at the same local time every day. Supports 24-hour (`HHMMSS` numeric format, e.g., `143000`, no quotes) and 12-hour (`"HH:MM:SS AM/PM"`, `"A/P"` string format with quotes).
    ```basic
    10 ON ALARM$(143000) GOSUB 200     : REM trap daily alarm at 2:30 PM (no quotes)
    20 SET ALARM$(143000) ON           : REM start/activate daily alarm
    30 ALARM$ ON                       : REM enable global daily alarms check
    40 PRINT ALARM$(143000)            : REM remaining time as "HH:MM:SS"
    200 PRINT "Daily alarm fired!" : RETURN
    ```
    Modifiers:
    - `SET ALARM$(time) ON`: Start or resume the daily alarm.
    - `SET ALARM$(time) OFF` / `STOP` / `PAUSE`: Stop or pause the daily alarm.
    - `SET ALARM$(time) SNOOZE [secs]`: Snooze the daily alarm (triggers again in `secs` seconds, defaults to 300).
    - `SET ALARM$(time) UNSET`: Completely delete and clear the daily alarm.
    - `ALARM$ ON / OFF / STOP` controls the global daily alarms state.

### 3.5 Device I/O Events (VDev Tier 2)

Any registered virtual device (VDev) with the `VDCAP_EVENT` capability can
fire events.  This covers USB hotplug, printer status changes, mouse input,
and disk I/O completion.

Device events are polled automatically by `event_poll()`.  When a device's
`dev_poll()` returns 1 (event pending) and its trap state is ON, the
registered handler fires.

```
Conceptual syntax (planned):
  ON DEVICE(n) GOSUB line      n = VDev slot number
  DEVICE(n) ON | OFF | STOP
```

The device event infrastructure is in place.  Specific device-level
`ON DEVICE` syntax will be exposed in a future version when individual
VDev drivers implement `dev_poll()`.

### 3.5b ON FILEIO — File I/O Events

Intercept file I/O completion or error events:

```basic
10 ON FILEIO GOSUB 800
20 FILEIO ON
30 REM ... program continues ...
800 PRINT "File I/O event fired"
810 RETURN
```

Internally, the runtime tracks:
- `on_fileio_line` — GOSUB target line number (0 = disabled)
- `fileio_event_state` — ON/OFF/STOP (same three-state as TIMER)
- `fileio_pending` — set to 1 when a disk event occurs

### 3.5c Event Queue System

When an event fires while its state is STOP (suspended), the event
is queued instead of being discarded.  When the state changes back
to ON, queued events fire in order.

The event queue is a fixed-size circular buffer:

```c
#define EVENT_QUEUE_SIZE 16
struct {
    int event_type;   /* EVTYPE_xxx (1-9) */
    int event_id;     /* device index or key number */
    int handler_line; /* target GOSUB line */
} event_queue[EVENT_QUEUE_SIZE];
int evq_head, evq_tail, evq_count;
```

Event type constants (from runtime.h):

| Constant       | Value | Event Source              |
|----------------|-------|---------------------------|
| `EVTYPE_TIMER` | 1     | ON TIMER                  |
| `EVTYPE_KEY`   | 2     | ON KEY                    |
| `EVTYPE_COM`   | 3     | ON COM                    |
| `EVTYPE_PEN`   | 4     | ON PEN                    |
| `EVTYPE_PLAY`  | 5     | ON PLAY                   |
| `EVTYPE_STRIG` | 6     | ON STRIG                  |
| `EVTYPE_DEVICE`| 7     | ON DEVICE                 |
| `EVTYPE_BREAK` | 8     | ON BREAK                  |
| `EVTYPE_FILEIO`| 9     | ON FILEIO                 |

### 3.6 UPnP / Network Events (Tier 5)

The UPnP module (`MODULE "UPNP"`) registers `UPNP:` and `SOAP:` virtual
devices with `VDCAP_EVENT`.  These are polled by the same Tier 2 device
event loop.

**Event types:**
- SSDP discovery announcement (new device found)
- GENA event notification (device state change)
- Connection state changes

```basic
100 MODULE "UPNP"
110 REM UPNP: and SOAP: devices are now registered
120 REM Events fire through the device I/O event system
```

The UPnP module is currently a pure API framework.  The SSDP multicast
and GENA subscription protocols have documented extension points for
future implementation.

---

## 4. ON TRAP — The Hybrid Event Router

`ON TRAP` is BASIC++'s unified syntax for registering handlers for
arbitrary event classes.  It subsumes `ON ERROR GOTO` and the
device-specific traps.

### 4.1 Syntax

```
ON TRAP GOTO line           Set error trap (same as ON ERROR GOTO)
ON TRAP GOSUB line          Set error trap as subroutine
ON TRAP(n) GOTO line        Set trap for event class n
ON TRAP(n) GOSUB line       Set trap for event class n as subroutine
```

### 4.2 Event Classes

| Class | Name    | Description                                |
|-------|---------|--------------------------------------------|
| 0     | ERROR   | Runtime error (division by zero, etc.)     |
| 1     | TIMER   | Timer interval elapsed                     |
| 2     | KEY     | Function key pressed                       |
| 3     | COM     | Serial port data available                 |
| 4     | PEN     | Light pen activated                        |
| 5     | STRIG   | Joystick button pressed                    |
| 6     | PLAY    | Music buffer needs filling                 |
| 7     | DEVICE  | Device I/O event (VDev Tier 2)             |
| 8     | BREAK   | OS signal / Ctrl+C (SIGINT, SIGTERM)       |
| 9     | FILEIO  | File I/O completion event                  |

### 4.3 Examples

**Error trapping via ON TRAP:**
```basic
10 ON TRAP GOTO 500
20 X = 1/0
30 END

500 PRINT "Trapped error"; ERR; "at line"; ERL
510 RESUME NEXT
```

**Timer event with ON TRAP(1):**
```basic
10 ON TRAP(1) GOSUB 500
20 FOR I = 1 TO 1000000
30   REM main loop
40 NEXT I
50 END

500 PRINT "Timer tick at"; TIME$
510 RETURN
```

### 4.4 Dialect Conflicts

In **Atari BASIC**, `TRAP` is a standalone statement (`TRAP n`), not
part of the `ON ... GOTO` family.  BASIC++ resolves this by
recognizing both forms:

| Input              | Atari Mode              | Other Dialects          |
|--------------------|-------------------------|-------------------------|
| `TRAP 100`         | Set error trap → line 100 | Same (compatible)      |
| `TRAP 0`           | Disable error trap       | Same                   |
| `TRAP`             | Show trap status         | Same                   |
| `TRAP 1, 0, 200`   | *(not valid)*           | Event 1, source 0, → 200 |
| `ON TRAP GOTO 100` | *(not standard)*        | Set error trap → 100    |
| `ON TRAP(1) GOSUB 500` | *(not standard)*   | Timer trap → 500        |

When running in Atari mode (`DIALECT ATAR`), the standalone `TRAP n`
form maps directly to `on_error_line`.  The extended forms
(`ON TRAP`, `TRAP event,source,dest`) are still available but would
not be idiomatic Atari BASIC.

In **mixed mode** (`OPTION MIXED ATAR,GWBS`), both Atari's `TRAP n`
and GW-BASIC's `ON ERROR GOTO n` coexist — they write to the same
internal handler slot (`on_error_line`), so they are interchangeable.

---

## 5. The TRAP Three-Argument Form

For explicit event routing without the `ON` prefix:

```
TRAP event_class, source, handler_line
```

This is the extended BASIC++ form that routes a specific event class
from a specific source to a handler.

```basic
10 TRAP 0, 0, 500      : REM error trap → line 500
20 TRAP 1, 0, 600      : REM timer trap → line 600
30 TRAP 2, 1, 700      : REM key trap, key 1 → line 700
```

**Arguments:**
- **event_class** — numeric event class (see table in §4.2)
- **source** — sub-index (port number, key number, etc.; 0 = default)
- **handler_line** — target line number (0 = disable)

---

## 6. Error Codes (ERR)

When a trap fires due to an error, the `ERR` variable holds the
error code.  Common codes:

| ERR Code | Meaning                          |
|----------|----------------------------------|
| 1        | NEXT without FOR                 |
| 2        | Syntax error                     |
| 3        | RETURN without GOSUB             |
| 4        | Out of DATA                      |
| 5        | Illegal function call            |
| 6        | Overflow                         |
| 7        | Out of memory                    |
| 8        | Undefined line number            |
| 9        | Subscript out of range           |
| 11       | Division by zero                 |
| 13       | Type mismatch                    |
| 53       | File not found                   |
| 57       | Device I/O error                 |
| 70       | Permission denied                |
| 77       | Advanced feature not available   |

See Error_Handling.md for the complete error code table (codes 1-76, plus user-defined 200-255).

The `ERL` variable holds the line number where the error occurred.

---

## 7. Comparison of Trapping Styles

| Feature                | ON ERROR GOTO   | TRAP n     | WHEN/USE   | ON TRAP     |
|------------------------|-----------------|------------|------------|-------------|
| **Dialect**            | GW-BASIC, QBasic| Atari BASIC| ECMA-116   | BASIC++     |
| **Scope**              | Global          | Global     | Block-local| Per-event   |
| **Resume options**     | RESUME / NEXT   | *(limited)*| RETRY/CONT | RETURN      |
| **Multiple handlers**  | No              | No         | Nested     | Yes (by class)|
| **Subroutine style**   | No (GOTO only)  | No         | N/A        | Yes (GOSUB) |
| **Works in all modes** | Yes             | Yes        | E116+      | Yes         |

---

## 8. Best Practices

1. **Use `WHEN EXCEPTION IN` for complex error recovery.**
   It provides block-scoped, nestable exception handling.

2. **Use `ON ERROR GOTO` for simple programs.**
   A single global handler is often sufficient.

3. **Use `ON TRAP(n) GOSUB` for event-driven programs.**
   It allows separate handlers for different event classes and uses
   GOSUB/RETURN for clean stack management.

4. **Always clear your trap when done:**
   ```basic
   ON ERROR GOTO 0     : REM disable error trap
   TRAP 0              : REM equivalent
   ```

5. **Test your handlers.**
   Use `ERROR n` or `CAUSE EXCEPTION n` to deliberately trigger errors:
   ```basic
   10 ON ERROR GOTO 100
   20 ERROR 99           : REM simulate error code 99
   30 END
   100 PRINT "Handler caught ERR ="; ERR
   110 RESUME NEXT
   ```

---

## 9. Quick Reference

```
ON ERROR GOTO n         Global error handler (GW-BASIC)
ON ERROR GOTO 0         Disable error handler
TRAP n                  Set error trap (Atari BASIC)
TRAP 0                  Disable error trap
TRAP                    Show current trap status
TRAP e, s, n            Extended event routing (BASIC++)
ON TRAP GOTO n          Error trap via ON syntax
ON TRAP GOSUB n         Error trap as subroutine
ON TRAP(n) GOTO line    Event class n trap
ON TRAP(n) GOSUB line   Event class n trap (subroutine)
WHEN EXCEPTION IN       Start protected block (ECMA-116)
USE                     Start exception handler
END WHEN                End exception block
RETRY                   Re-enter protected block
CONTINUE                Skip failed statement
RESUME                  Retry the failed line
RESUME NEXT             Continue after failed line
RESUME n                Jump to line n
ERROR n                 Simulate error code n
CAUSE EXCEPTION n       Raise user exception
ERR                     Last error code (read-only)
ERL                     Last error line (read-only)
ON BREAK GOSUB line     Intercept Ctrl+C / OS signal
ON BREAK GOTO 0         Restore default Ctrl+C behavior
ON FILEIO GOSUB line    Intercept file I/O events
FILEIO ON | OFF | STOP  Enable/disable/suspend FILEIO events
MODULE "UPNP"           Activate UPnP device discovery
```
