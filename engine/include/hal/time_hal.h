// FILENAME: time_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for system timers and calendar time.
//
// ---- Includes ----

#ifndef HAL_TIME_HAL_H
#define HAL_TIME_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TimeHal {
    // Current UTC epoch timestamp in seconds
    int64_t  (*now_epoch_seconds)(void);

    // Monotonic system uptime in milliseconds (wrapping safe)
    uint64_t (*monotonic_ms)(void);

    // High-resolution performance counter ticks
    uint64_t (*highres_ticks)(void);

    // Frequency of highres ticks per second (Hz)
    uint64_t (*ticks_frequency)(void);

    // Sleep / delay current thread/task by milliseconds
    void     (*sleep_ms)(uint32_t ms);
} TimeHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_TIME_HAL_H
