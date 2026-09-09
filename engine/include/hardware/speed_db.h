// FILENAME: speed_db.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine
// Vintage folklore system speed database and catalog definitions.

#ifndef SPEED_DB_H
#define SPEED_DB_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SPEED_DB_MAX_ALIASES 6
#define SPEED_DB_MAX_CUSTOM 32

typedef struct {
    char model[48];
    int year;
    double cpu_hz;
    double cpu_mhz;
    char cpu_arch[24];
    double bus_hz;
    double bus_mhz;
    char bus_type[24];
    char aliases[SPEED_DB_MAX_ALIASES][32];
    int alias_count;
} LegacySystemSpec;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize or reset database (static base catalog of 20 IBM systems)
void speed_db_init(void);

// Find a system specification by name or alias (case-insensitive)
const LegacySystemSpec *speed_db_find(const char *name);

// Find a system specification by approximate CPU MHz (within 1%)
const LegacySystemSpec *speed_db_find_by_mhz(double mhz);

// Count total available systems (base + custom)
int speed_db_count(void);

// Get system specification by 0-based index
const LegacySystemSpec *speed_db_get_at(int index);

// Add or override a custom system specification at runtime (thread-safe, zero bare malloc)
bool speed_db_add_custom(const LegacySystemSpec *spec);

// Format Hz value to human-readable string (e.g. "4.77 MHz", "3.40 GHz")
void speed_db_format_unit(double hz, char *buf, size_t buf_sz);

// Calculate relative speed multiplier (host_hz / target_hz)
double speed_db_calc_ratio(double host_hz, double target_hz);

#ifdef __cplusplus
}
#endif

#endif // SPEED_DB_H
