// FILENAME: bgi_bridge.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (bgi_bridge.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual device and graphics rendering logic for bgi_bridge.
//
// ---- Includes ----

#ifndef ENGINE_DEVICE_BGI_BRIDGE_H
#define ENGINE_DEVICE_BGI_BRIDGE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bios/bios.h"

#ifdef __cplusplus
extern "C" {
#endif

bool bgi_bridge_init(BiosContext* bios);
void bgi_bridge_shutdown(BiosContext* bios);
void bgi_bridge_on_vram_write(void* user_data, uint32_t phys_addr, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_DEVICE_BGI_BRIDGE_H
