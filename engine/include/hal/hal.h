// FILENAME: hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libhardware, libkernel, libserver, libstandard
// NEEDS: libadvanced (audio_hal.h)
// NEEDS: libcore (input_hal.h, io_hal.h, mem_hal.h, time_hal.h, video_hal.h)
// Master Hardware Abstraction Layer interface definitions.
//
// ---- Includes ----

#ifndef HAL_HAL_H
#define HAL_HAL_H

#include "hal/mem_hal.h"
#include "hal/io_hal.h"
#include "hal/time_hal.h"
#include "hal/audio_hal.h"
#include "hal/video_hal.h"
#include "hal/input_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Master HAL Table aggregating all subsystem interfaces
typedef struct HalContext {
    MemHal   mem;
    IoHal    io;
    TimeHal  time;
    AudioHal audio;
    VideoHal video;
    InputHal input;
} HalContext;

// @brief Retrieve the global active HAL context.
HalContext *hal_get(void);

// @brief Set or replace the global active HAL context.
void hal_set(const HalContext *ctx);

// @brief Initialize default hosted HAL (mapping to standard OS platform abstractions).
void hal_init_hosted(void);

// @brief Initialize pure freestanding HAL (null/stub fallbacks with static pool allocator).
void hal_init_freestanding(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_HAL_H
