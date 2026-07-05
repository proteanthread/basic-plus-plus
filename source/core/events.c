/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Event trapping (KEY, COM, TIMER) management and background event dispatcher.
 *    - Event loop integration for keyboard, serial port, and timer tick trapping.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#include "events.h"
#include <stdio.h>

#ifndef NO_SDL2
#include <SDL.h>
#else
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#include "../console.h"
#include "../console.h"
#endif
#endif

void gw_event_register_key(GW_State *state, int key_idx, int32_t gosub_line) {
    if (!state || key_idx < 0 || key_idx >= NMKEYF) return;
    events_core_register_trap(&state->key_traps[key_idx], gosub_line);
}

void gw_event_register_timer(GW_State *state, int timer_idx, int32_t gosub_line) {
    if (!state || timer_idx < 0 || timer_idx >= 4) return;
    events_core_register_trap(&state->timer_traps[timer_idx], gosub_line);
}

static uint32_t g_last_ticks[4] = {0, 0, 0, 0};

static uint32_t get_ticks_ms(void) {
#ifndef NO_SDL2
    return SDL_GetTicks();
#else
#ifdef _WIN32
    return GetTickCount();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
#endif
}

void gw_event_poll(GW_State *state) {
    if (!state) return;
    
    uint32_t current_ticks = get_ticks_ms();
    
    // Poll Timer traps
    events_core_poll_timer(state->timer_traps, 4, current_ticks, g_last_ticks);
    
    // Check if any trapped event can trigger a GOSUB branch
    for (int i = 0; i < 4; i++) {
        if (state->timer_traps[i].trapped && !state->timer_traps[i].active && !state->in_error_handler) {
            state->timer_traps[i].trapped = false;
            state->timer_traps[i].active = true;
            
            printf("[Event Trap: Timer %d Branching to %d]\n", i + 1, state->timer_traps[i].gosub_line);
            // In a fully recursive statement executor, we would trigger a branch here.
        }
    }
}
