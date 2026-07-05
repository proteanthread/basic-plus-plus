/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: events_core.h
 * Subsystem: Event Trapping Interrupt State Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Tracks event handlers (ON KEY, ON PLAY, ON TIMER) and schedules GOSUB interrupts.
 *
 * 2. WHAT TO EXPECT:
 *    Maintains trigger flags and routes execution hooks.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Interrupt latency settings, keyboard scanning codes.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Execution dispatcher tick routing.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If event triggers fail to run, check lock levels and interrupt flags.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE COOPERATIVE EVENT TRAPPING CORE
 * File: events_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_EVENTS_CORE_H
#define BASICPP_STANDALONE_EVENTS_CORE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct BppEventTrapCore {
    bool    enabled;
    bool    active;
    bool    trapped;
    int32_t gosub_line;
} BppEventTrapCore;

void events_core_register_trap(BppEventTrapCore *trap, int32_t gosub_line);
void events_core_poll_timer(BppEventTrapCore *traps, int max_traps, uint32_t current_ticks, uint32_t *last_ticks);

#endif // BASICPP_STANDALONE_EVENTS_CORE_H
