// FILENAME: bios_hal_vm.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libhardware (bios_hal.h)
// Implements bytecode virtual machine execution and state for bios_hal_vm.
//
// ---- Includes ----

#include "bios/bios_hal.h"
#include "runtime/string/memops.h"

static uint8_t default_port_in8(void* user_data, uint16_t port) {
    (void)user_data;
    (void)port;
    return 0xFF; // Default bus float value for unmapped ports
}

static void default_port_out8(void* user_data, uint16_t port, uint8_t val) {
    (void)user_data;
    (void)port;
    (void)val;
}

static uint16_t default_port_in16(void* user_data, uint16_t port) {
    (void)user_data;
    (void)port;
    return 0xFFFF;
}

static void default_port_out16(void* user_data, uint16_t port, uint16_t val) {
    (void)user_data;
    (void)port;
    (void)val;
}

static uint8_t default_mem_read8(void* user_data, uint32_t phys_addr) {
    (void)user_data;
    (void)phys_addr;
    return 0x00;
}

static void default_mem_write8(void* user_data, uint32_t phys_addr, uint8_t val) {
    (void)user_data;
    (void)phys_addr;
    (void)val;
}

static void default_assert_irq(void* user_data, uint8_t irq_num) {
    (void)user_data;
    (void)irq_num;
}

static void default_tick_timer(void* user_data, uint32_t elapsed_us) {
    (void)user_data;
    (void)elapsed_us;
}

void bios_hal_init_default(BiosHAL* hal, void* user_data) {
    if (!hal) return;
    memset(hal, 0, sizeof(BiosHAL));
    hal->user_data   = user_data;
    hal->port_in8    = default_port_in8;
    hal->port_out8   = default_port_out8;
    hal->port_in16   = default_port_in16;
    hal->port_out16  = default_port_out16;
    hal->mem_read8   = default_mem_read8;
    hal->mem_write8  = default_mem_write8;
    hal->assert_irq  = default_assert_irq;
    hal->tick_timer  = default_tick_timer;
}
