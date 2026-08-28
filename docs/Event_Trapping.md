# BASIC++ v6.5.2 Event Trapping

## 1. THE EVENT SYSTEM

BASIC++ provides an event trapping system that monitors hardware and software events asynchronously during program execution. When a trapped event occurs, the VM suspends normal execution and transfers control to a user-defined subroutine. After the subroutine completes with RETURN, normal execution resumes at the point where it was interrupted.

The event system is implemented in engine/src/vm/events.c. Event polling occurs between statement executions — the VM checks the event queue after each statement completes. This means events are not truly preemptive; they are checked at statement boundaries.

## 2. EVENT TRAPPING SYNTAX

All event traps follow a three-part pattern:

1. **Declare the handler**: `ON event GOSUB line` — specifies which subroutine handles the event.
2. **Enable the trap**: `event ON` — activates event monitoring.
3. **Process the event**: The subroutine at the specified line executes and ends with RETURN.

The event can be disabled with `event OFF` (stops monitoring) or suspended with `event STOP` (queues events but does not fire handlers until re-enabled with `event ON`).

## 3. KEY TRAPPING

ON KEY(n) GOSUB line traps a specific key press. Key numbers 1-10 correspond to function keys F1-F10. Key numbers 11-14 correspond to the arrow keys. Key numbers 15-25 are user-definable with the KEY statement.

```basic
10 ON KEY(1) GOSUB 1000    ' Trap F1
20 KEY(1) ON                ' Enable trapping
30 PRINT "Press F1 to see help..."
40 FOR I = 1 TO 10000 : NEXT I
50 KEY(1) OFF
60 END
1000 PRINT "Help: This is a demo program"
1010 RETURN
```

KEY(n) ON enables trapping. KEY(n) OFF disables it. KEY(n) STOP suspends trapping but queues the event — when KEY(n) ON is re-issued, the queued event fires immediately.

User-definable key traps use the KEY statement to assign a scan code and shift-state mask: `KEY 15, CHR$(shift) + CHR$(scancode)`. Then `ON KEY(15) GOSUB line` and `KEY(15) ON` trap that specific key combination.

## 4. TIMER TRAPPING

ON TIMER(n) GOSUB line fires the handler every n seconds:

```basic
10 ON TIMER(5) GOSUB 1000   ' Every 5 seconds
20 TIMER ON
30 WHILE INKEY$ = "" : WEND
40 TIMER OFF
50 END
1000 PRINT TIME$; " - Timer fired"
1010 RETURN
```

The timer resolution is approximately 1/18.2 seconds (one PIT tick) on FreeDOS builds and millisecond-accurate on modern builds. The minimum interval is 1 second.

TIMER ON enables the timer trap. TIMER OFF disables it. TIMER STOP suspends it.

## 5. COM PORT TRAPPING

ON COM(n) GOSUB line traps incoming data on serial port n (1 or 2):

```basic
10 ON COM(1) GOSUB 2000
20 COM(1) ON
```

When data arrives at the serial port, the handler fires. Inside the handler, INPUT #n reads the received data. COM(n) OFF disables trapping. COM(n) STOP suspends it.

## 6. PEN TRAPPING

ON PEN GOSUB line traps light pen activation:

```basic
10 ON PEN GOSUB 3000
20 PEN ON
```

Inside the handler, PEN(0) through PEN(9) return pen coordinates and status. PEN trapping is primarily relevant for retro hardware emulation.

## 7. STRIG TRAPPING (JOYSTICK)

ON STRIG(n) GOSUB line traps joystick button presses. Button numbers 0-3 correspond to two joystick buttons on two joysticks:

```basic
10 ON STRIG(0) GOSUB 4000    ' Joystick 1, Button 1
20 STRIG(0) ON
```

STICK(0) through STICK(3) return joystick axis positions.

## 8. PLAY TRAPPING

ON PLAY(n) GOSUB line fires when the music buffer drops below n notes:

```basic
10 ON PLAY(3) GOSUB 5000
20 PLAY ON
30 PLAY "L4 CDEFGAB"
```

This allows continuous music playback by refilling the buffer when it runs low.

## 9. ALARM TRAPPING

BASIC++ extends the classic event system with alarm traps. The alarm system supports both countdown timers and daily scheduled alarms:

```basic
10 ON ALARM GOSUB 6000
20 ALARM "14:30:00"          ' Daily alarm at 2:30 PM
30 ALARM ON
```

ALARM with a time string sets a daily alarm. ALARM with a numeric value sets a countdown in seconds: `ALARM 300` fires in 5 minutes.

ALARM$ returns the scheduled alarm time as a string. SNOOZE n postpones the alarm by n seconds. UNSET cancels a pending alarm.

## 10. MOUSE TRAPPING

ON MOUSE GOSUB line traps mouse button clicks and movement:

```basic
10 ON MOUSE GOSUB 7000
20 MOUSE ON
```

Inside the handler, HMOUSE returns the horizontal position, VMOUSE returns the vertical position, and TRIG returns the button state.

ON HMOUSE GOSUB line traps horizontal movement. ON VMOUSE GOSUB line traps vertical movement. ON TRIG GOSUB line traps button presses specifically.

## 11. NESTED AND PRIORITIZED EVENTS

When an event fires while another event handler is executing, the new event is queued. Queued events fire in order after the current handler returns. Events of the same type are coalesced — multiple timer events that occur during a handler execution result in a single handler call, not multiple.

The event processing order when multiple events are pending: KEY, TIMER, COM, PEN, STRIG, PLAY, ALARM, MOUSE.

## 12. EVENT STATE TRANSITIONS

Each event type has three states:

- **OFF** — Not monitored. Events are ignored.
- **ON** — Actively monitored. Events fire the handler immediately at the next statement boundary.
- **STOP** — Monitored but suspended. Events are recorded but handlers do not fire. When the state changes from STOP to ON, any pending event fires immediately.

The STOP state is useful inside event handlers to prevent re-entrant handler calls: set the event to STOP at the start of the handler, process the event, then set it back to ON before RETURN.
