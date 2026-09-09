// FILENAME: speed_emulate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine, libkernel
// Vintage folklore execution and I/O pacing emulation engine.

#ifndef SPEED_EMULATE_H
#define SPEED_EMULATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize or reset emulation state
void speed_emulate_init(void);

// Set target emulation system by model name or alias
bool speed_emulate_set_target(const char *target_name, bool throttle_io);

// Set target emulation system by explicit MHz
bool speed_emulate_set_mhz(double mhz, bool throttle_io);

// Enable or disable emulation
void speed_emulate_set_active(bool active);

// Query whether emulation is currently active
bool speed_emulate_is_active(void);

// Query whether I/O throttling is currently active
bool speed_emulate_is_io_active(void);

// Get currently emulated system model name (or empty string if none)
const char *speed_emulate_get_target(void);

// Get target frequency in Hz
double speed_emulate_get_target_hz(void);

// Get current relative speed multiplier (host / target)
double speed_emulate_get_ratio(void);

// Statement execution pacing hook (called from VM statement loop)
void speed_emulate_pace_statement(void);

// I/O pacing hook (called from console output and file operations)
void speed_emulate_pace_io(size_t bytes);

#ifdef __cplusplus
}
#endif

#endif // SPEED_EMULATE_H
