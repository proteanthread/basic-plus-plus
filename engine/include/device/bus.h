// FILENAME: bus.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: platform, memory
// Implements virtual device and graphics rendering logic for bus.
//
// ---- Includes ----

#ifndef DEVICE_BUS_H
#define DEVICE_BUS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BIOS_MODEL_NONE = 0,
    BIOS_MODEL_MSDOS,
    BIOS_MODEL_IBMPC,
    BIOS_MODEL_PCJR,
    BIOS_MODEL_PCXT,
    BIOS_MODEL_PCAT,
    BIOS_MODEL_APPLE2,
    BIOS_MODEL_C64,
    BIOS_MODEL_VIC20,
    BIOS_MODEL_ATARI,
    BIOS_MODEL_TANDY
} MockBiosModel;

void vdev_bus_out(int port, int value);
int  vdev_bus_in(int port);

uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted);
void    vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted);

// Port handler registration
bool vdev_bus_register_port(int start, int end, int (*read_fn)(int), void (*write_fn)(int, int));
void vdev_bus_reset(void);

// Mock BIOS linkage functions
void vdev_bus_set_model(MockBiosModel model);
MockBiosModel vdev_bus_get_model(void);
void vdev_bus_set_ram(uint8_t *ram, size_t size);

#endif // DEVICE_BUS_H
