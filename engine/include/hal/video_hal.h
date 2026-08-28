// FILENAME: video_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for display presentation and framebuffer.
//
// ---- Includes ----

#ifndef HAL_VIDEO_HAL_H
#define HAL_VIDEO_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VideoHal {
    bool (*init)(int width, int height, int bpp, const char *title);
    void (*shutdown)(void);
    void (*present_framebuffer)(const void *pixels, int width, int height, int pitch);
    void (*set_palette)(const uint32_t *palette, int count);
    void (*poll_events)(void);
    bool (*is_window_open)(void);
} VideoHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_VIDEO_HAL_H
