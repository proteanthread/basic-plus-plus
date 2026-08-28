// FILENAME: input_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for input, keyboard, and mouse.
//
// ---- Includes ----

#ifndef HAL_INPUT_HAL_H
#define HAL_INPUT_HAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct InputHal {
    // Poll single keypress (non-blocking). Returns true if key available.
    bool (*poll_key)(int *out_key);

    // Poll mouse position and button state
    bool (*poll_mouse)(int *out_x, int *out_y, int *out_buttons);

    // Check if a specific physical key is currently held down
    bool (*is_key_down)(int scancode);
} InputHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_INPUT_HAL_H
