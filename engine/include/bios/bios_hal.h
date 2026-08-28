// FILENAME: bios_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (bios_hal_vm.c)
// NEEDS: platform, memory
// Implements virtual BIOS interrupt and hardware emulation for bios_hal.
//
// ---- Includes ----

#ifndef ENGINE_BIOS_HAL_H
#define ENGINE_BIOS_HAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------
// HAL Operating Modes
// -------------------------------------------------------------------------
#define BIOS_HAL_MODE_STATIC_INLINE    1  // Direct inline port/mem for IoT/MCU
#define BIOS_HAL_MODE_PLUGGABLE_STRUCT 2  // Function pointers for emulators
#define BIOS_HAL_MODE_MACRO_OVERRIDE   3  // Macro wrappers for FreeDOS/UEFI

#ifndef BIOS_HAL_MODE
#define BIOS_HAL_MODE BIOS_HAL_MODE_PLUGGABLE_STRUCT
#endif

// -------------------------------------------------------------------------
// Pluggable HAL Function Pointer Types (Mode 2)
// -------------------------------------------------------------------------
typedef uint8_t  (*BiosHalPortIn8Fn)(void* user_data, uint16_t port);
typedef void     (*BiosHalPortOut8Fn)(void* user_data, uint16_t port, uint8_t val);
typedef uint16_t (*BiosHalPortIn16Fn)(void* user_data, uint16_t port);
typedef void     (*BiosHalPortOut16Fn)(void* user_data, uint16_t port, uint16_t val);

typedef uint8_t  (*BiosHalMemRead8Fn)(void* user_data, uint32_t phys_addr);
typedef void     (*BiosHalMemWrite8Fn)(void* user_data, uint32_t phys_addr, uint8_t val);

typedef void     (*BiosHalAssertIRQFn)(void* user_data, uint8_t irq_num);
typedef void     (*BiosHalTickTimerFn)(void* user_data, uint32_t elapsed_us);

typedef struct {
    void*               user_data;
    BiosHalPortIn8Fn    port_in8;
    BiosHalPortOut8Fn   port_out8;
    BiosHalPortIn16Fn   port_in16;
    BiosHalPortOut16Fn  port_out16;
    BiosHalMemRead8Fn   mem_read8;
    BiosHalMemWrite8Fn  mem_write8;
    BiosHalAssertIRQFn  assert_irq;
    BiosHalTickTimerFn  tick_timer;
} BiosHAL;

// -------------------------------------------------------------------------
// Public HAL Helper Prototypes
// -------------------------------------------------------------------------
void bios_hal_init_default(BiosHAL* hal, void* user_data);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_BIOS_HAL_H
