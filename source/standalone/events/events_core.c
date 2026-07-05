/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: events_core.c
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
 * File: events_core.c
 * ===================================================================== */

#include "events_core.h"

void events_core_register_trap(BppEventTrapCore *trap, int32_t gosub_line)
{
    if (trap) {
        trap->gosub_line = gosub_line;
        trap->enabled = (gosub_line > 0);
        trap->active = false;
        trap->trapped = false;
    }
}

void events_core_poll_timer(BppEventTrapCore *traps, int max_traps, uint32_t current_ticks, uint32_t *last_ticks)
{
    if (traps && last_ticks && max_traps > 0) {
        int i;
        for (i = 0; i < max_traps; i++) {
            if (traps[i].enabled) {
                uint32_t interval = (uint32_t)(i + 1) * 1000;
                if (current_ticks - last_ticks[i] >= interval) {
                    traps[i].trapped = true;
                    last_ticks[i] = current_ticks;
                }
            }
        }
    }
}
