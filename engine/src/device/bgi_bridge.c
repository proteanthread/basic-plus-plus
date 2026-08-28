// FILENAME: bgi_bridge.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libkernel (bgi_bridge.h, bgi_gfx.h, bgi_gfx.c)
// Implements virtual device and graphics rendering logic for bgi_bridge.
//
// ---- Includes ----

#include "device/bgi_bridge.h"
#include "device/bgi_gfx.h"
#include "runtime/string/memops.h"

void bgi_bridge_on_vram_write(void* user_data, uint32_t phys_addr, uint8_t val) {
    (void)user_data;
    (void)phys_addr;
    (void)val;
    // Direct VRAM sync hook for BGI active rasterizer
}

bool bgi_bridge_init(BiosContext* bios) {
    if (!bios) return false;

    BiosVRAMObserver observer;
    runtime_memset(&observer, 0, sizeof(observer));
    observer.user_data = bios;
    observer.on_write   = bgi_bridge_on_vram_write;
    observer.on_read    = NULL;

    bios_set_vram_observer(bios, &observer);
    return true;
}

void bgi_bridge_shutdown(BiosContext* bios) {
    if (!bios) return;
    bios_set_vram_observer(bios, NULL);
}

