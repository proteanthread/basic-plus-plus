// FILENAME: events.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (events_internal.h)
// Implements bytecode virtual machine execution and state for events.
//
// ---- Includes ----

#include "vm/events_internal.h"

// All asynchronous event trapping implementation is decomposed into:
// - vm/events/events_trap.c: Trap registration and state setters/getters
// - vm/events/events_alarm.c: Countdown and daily scheduled alarms
// - vm/events/events_poll.c: Event polling loop and handler dispatching
