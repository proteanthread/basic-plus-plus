// FILENAME: bios_cpu8086.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware (bios_cpu8086.c)
// NEEDS: platform, memory
// Implements virtual BIOS interrupt and hardware emulation for bios_cpu8086.
//
// ---- Includes ----

#ifndef ENGINE_BIOS_CPU8086_H
#define ENGINE_BIOS_CPU8086_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t sp;
    uint16_t ip;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
    uint16_t flags;
    bool     halted;
} BiosCpu8086Context;

void bios_cpu8086_reset(BiosCpu8086Context* cpu);
bool bios_cpu8086_step(BiosCpu8086Context* cpu, uint8_t* phys_mem_1mb);
void bios_cpu8086_int(BiosCpu8086Context* cpu, uint8_t int_num, uint8_t* phys_mem_1mb);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_BIOS_CPU8086_H
