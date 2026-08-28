// FILENAME: bios_at.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDED BY: libhardware (bios_at.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_at.
//
// ---- Includes ----

#ifndef ENGINE_BIOS_BIOS_AT_H
#define ENGINE_BIOS_BIOS_AT_H

#include "bios/bios.h"

#ifdef __cplusplus
extern "C" {
#endif

void bios_at_init(BiosContext* ctx);
uint8_t bios_at_inp(BiosContext* ctx, uint16_t port);
void bios_at_out(BiosContext* ctx, uint16_t port, uint8_t val);
bool bios_at_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_BIOS_BIOS_AT_H
