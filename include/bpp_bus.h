/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_bus.h
 * @brief Virtual Hardware Port & MMIO Bus interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares routing callbacks for INP/OUT port instructions and
 *   PEEK/POKE conventional memory intercepts.
 * - Why it exists: Connects execution engines directly to emulated PC peripheral chips
 *   and BIOS structures.
 */

#ifndef BPP_BUS_H
#define BPP_BUS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bpp_mock_bios.h"

void vdev_bus_out(int port, int value);
int  vdev_bus_in(int port);

uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted);
void    vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted);

/* Port handler registration */
bool vdev_bus_register_port(int start, int end, int (*read_fn)(int), void (*write_fn)(int, int));
void vdev_bus_reset(void);

/* Mock BIOS linkage functions */
void vdev_bus_set_model(MockBiosModel model);
MockBiosModel vdev_bus_get_model(void);
void vdev_bus_set_ram(uint8_t *ram, size_t size);

#endif /* BPP_BUS_H */
