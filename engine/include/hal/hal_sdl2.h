// FILENAME: hal_sdl2.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal_hosted.c, hal_sdl2.c)
// NEEDS: libcore (hal.h)
// SDL2 driver implementations for Hardware Abstraction Layer.
//
// ---- Includes ----

#ifndef HAL_SDL2_H
#define HAL_SDL2_H

#include "hal/hal.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the SDL2 video and audio drivers and attaches them to the active HalContext.
void hal_init_sdl2(void);

// Video Driver Functions
bool sdl2_video_init(int width, int height, int bpp, const char *title);
void sdl2_video_shutdown(void);
void sdl2_video_present(const void *pixels, int width, int height, int pitch);
void sdl2_video_set_palette(const uint32_t *palette, int count);
void sdl2_video_poll_events(void);
bool sdl2_video_is_window_open(void);

// Audio Driver Functions
bool sdl2_audio_init(void);
void sdl2_audio_shutdown(void);
void sdl2_audio_beep(void);
void sdl2_audio_tone(uint32_t frequency_hz, uint32_t duration_ms);
void sdl2_audio_stop(void);
void sdl2_audio_set_volume(uint8_t volume);

#ifdef __cplusplus
}
#endif

#endif // HAL_SDL2_H
