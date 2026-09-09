// FILENAME: plat_hw_speed.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine
// Platform hardware speed and clock probing interfaces.

#ifndef PLAT_HW_SPEED_H
#define PLAT_HW_SPEED_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns host CPU frequency in Hz (e.g. 3400000000.0 for 3.4 GHz)
double plat_hw_get_cpu_hz(void);

// Returns host Bus frequency in Hz (e.g. 100000000.0 for 100 MHz reference clock)
double plat_hw_get_bus_hz(void);

// Returns host Memory clock in Hz (e.g. 3200000000.0 for 3200 MT/s DDR)
double plat_hw_get_mem_hz(void);

// Returns host GPU clock in Hz (e.g. 1500000000.0 for 1500 MHz)
double plat_hw_get_gpu_hz(void);

// Returns current hardware cycle/tick count (RDTSC / high-resolution timer)
uint64_t plat_hw_get_cycle_count(void);

// Run a fast freestanding calibration loop to calibrate frequency if OS queries are unavailable
void plat_hw_calibrate(void);

// User/runtime overrides for hardware clocks
void plat_hw_set_cpu_hz(double hz);
void plat_hw_set_bus_hz(double hz);
void plat_hw_set_mem_hz(double hz);
void plat_hw_set_gpu_hz(double hz);

// High-resolution microsecond sleep/delay for emulation pacing
void plat_hw_delay_us(uint64_t us);

#ifdef __cplusplus
}
#endif

#endif // PLAT_HW_SPEED_H
