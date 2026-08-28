// FILENAME: bios_xt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios.c)
// NEEDED BY: libhardware (bios_at.c, bios_xt.c)
// NEEDS: libengine (bios.h, bios.c)
// Implements virtual BIOS interrupt and hardware emulation for bios_xt.
//
// ---- Includes ----

#ifndef ENGINE_BIOS_BIOS_XT_H
#define ENGINE_BIOS_BIOS_XT_H

#include "bios/bios.h"

#ifdef __cplusplus
extern "C" {
#endif

void bios_xt_init(BiosContext* ctx);
uint8_t bios_xt_inp(BiosContext* ctx, uint16_t port);
void bios_xt_out(BiosContext* ctx, uint16_t port, uint8_t val);
bool bios_xt_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_BIOS_BIOS_XT_H
