// FILENAME: dev_framebuffer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libdevice, vdev.c
// NEEDS: libkernel (vdev.h)
// Header definitions for 4-bit / 8-bit Off-Screen Graphics Framebuffers (SCR1..8:).
//
// ---- Includes ----

#ifndef DEVICE_DEV_FRAMEBUFFER_H
#define DEVICE_DEV_FRAMEBUFFER_H

#include "device/vdev.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Creates a concrete VDev instance for an off-screen graphics canvas (SCR1..8:).
VDev dev_framebuffer_create(const char *name, int buffer_index);

// @brief Set a pixel in the specified canvas (0..255 palette index).
void dev_fb_pset(int buffer_index, int x, int y, uint8_t color);

// @brief Get a pixel from the specified canvas.
uint8_t dev_fb_pget(int buffer_index, int x, int y);

// @brief Cross-buffer rectangular blit between framebuffers.
void dev_fb_blit(int dst_buf, int dst_x, int dst_y, int src_buf, int src_x, int src_y, int w, int h);

#ifdef __cplusplus
}
#endif

#endif // DEVICE_DEV_FRAMEBUFFER_H
