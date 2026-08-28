// FILENAME: audio_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for audio and sound generation.
//
// ---- Includes ----

#ifndef HAL_AUDIO_HAL_H
#define HAL_AUDIO_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AudioHal {
    bool (*init)(void);
    void (*shutdown)(void);
    void (*beep)(void);
    void (*tone)(uint32_t frequency_hz, uint32_t duration_ms);
    void (*stop)(void);
    void (*set_volume)(uint8_t volume); // [0..100]
} AudioHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_AUDIO_HAL_H
