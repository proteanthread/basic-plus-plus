// FILENAME: bgi_text.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c)
// NEEDED BY: libkernel (bgi_text.c)
// NEEDS: libengine (bios.h, bios.c)
// NEEDS: libkernel (vcon.h, vcon.c)
// Implements virtual device and graphics rendering logic for bgi_text.
//
// ---- Includes ----

#ifndef DEVICE_BGI_TEXT_H
#define DEVICE_BGI_TEXT_H

#include "bios/bios.h"
#include "device/vcon.h"

// @brief Initialize text-mode VRAM observer.
//
// Registers a VRAM write observer on the given BiosContext that renders
// CGA/MDA text-mode writes to the host terminal and synchronizes the
// VCon grid buffer.
//
// @param bios   Active BiosContext (must remain valid for observer lifetime).
// @param vcon   Active VConContext (must remain valid for observer lifetime).
void bgi_text_init(BiosContext* bios, VConContext* vcon);

// @brief Shutdown text-mode VRAM observer.
//
// Unregisters the VRAM write observer from the BiosContext.
//
// @param bios   BiosContext to unregister from (may be NULL for safe shutdown).
void bgi_text_shutdown(BiosContext* bios);

#endif // DEVICE_BGI_TEXT_H
