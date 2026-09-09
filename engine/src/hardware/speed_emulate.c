// FILENAME: speed_emulate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine, libkernel
// Vintage folklore execution and I/O pacing emulation implementation.

#include "hardware/speed_emulate.h"
#include "hardware/speed_db.h"
#include "platform/plat_hw_speed.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int local_case_cmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 ? 1 : (s2 ? -1 : 0);
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static bool s_emulate_active = false;
static bool s_emulate_io = false;
static char s_emulate_target[48] = {0};
static double s_target_hz = 0.0;
static double s_emulate_ratio = 1.0;
static uint64_t s_stmt_delay_us = 0;
static uint64_t s_io_byte_delay_us = 0;
static uint32_t s_stmt_counter = 0;

void speed_emulate_init(void) {
    s_emulate_active = false;
    s_emulate_io = false;
    s_emulate_target[0] = '\0';
    s_target_hz = 0.0;
    s_emulate_ratio = 1.0;
    s_stmt_delay_us = 0;
    s_io_byte_delay_us = 0;
    s_stmt_counter = 0;
}

static void recalculate_pacing(void) {
    double host_hz = plat_hw_get_cpu_hz();
    if (s_target_hz <= 0.0) s_target_hz = 4772727.0; // Default to IBM PC 5150
    s_emulate_ratio = speed_db_calc_ratio(host_hz, s_target_hz);

    if (s_emulate_ratio > 1.0) {
        // Average statement execution on vintage 8088/286: ~150 cycles
        // Duration on target: 150 / target_hz seconds
        double stmt_target_sec = 150.0 / s_target_hz;
        double stmt_target_us = stmt_target_sec * 1000000.0;
        if (stmt_target_us < 1.0) stmt_target_us = 1.0;
        if (stmt_target_us > 10000.0) stmt_target_us = 10000.0;
        s_stmt_delay_us = (uint64_t)stmt_target_us;

        // Character I/O delay: e.g. 9600 bps = ~1040 us / char, or 8088 video bus write: ~100 us
        double io_target_us = (1000.0 / s_target_hz) * 1000000.0;
        if (io_target_us < 50.0) io_target_us = 50.0;
        if (io_target_us > 2000.0) io_target_us = 2000.0;
        s_io_byte_delay_us = (uint64_t)io_target_us;
    } else {
        s_stmt_delay_us = 0;
        s_io_byte_delay_us = 0;
    }
}

bool speed_emulate_set_target(const char *target_name, bool throttle_io) {
    if (!target_name || !*target_name) {
        s_emulate_active = false;
        return true;
    }

    if (local_case_cmp(target_name, "OFF") == 0 || local_case_cmp(target_name, "NONE") == 0) {
        s_emulate_active = false;
        return true;
    }

    const LegacySystemSpec *spec = speed_db_find(target_name);
    if (!spec) return false;

    strncpy(s_emulate_target, spec->model, sizeof(s_emulate_target) - 1);
    s_emulate_target[sizeof(s_emulate_target) - 1] = '\0';
    s_target_hz = spec->cpu_hz;
    s_emulate_io = throttle_io;
    s_emulate_active = true;
    recalculate_pacing();
    return true;
}

bool speed_emulate_set_mhz(double mhz, bool throttle_io) {
    if (mhz <= 0.0) {
        s_emulate_active = false;
        return true;
    }

    const LegacySystemSpec *spec = speed_db_find_by_mhz(mhz);
    if (spec) {
        strncpy(s_emulate_target, spec->model, sizeof(s_emulate_target) - 1);
    } else {
        snprintf(s_emulate_target, sizeof(s_emulate_target), "Custom %.2f MHz", mhz);
    }
    s_emulate_target[sizeof(s_emulate_target) - 1] = '\0';
    s_target_hz = mhz * 1000000.0;
    s_emulate_io = throttle_io;
    s_emulate_active = true;
    recalculate_pacing();
    return true;
}

void speed_emulate_set_active(bool active) {
    s_emulate_active = active;
    if (active && s_target_hz <= 0.0) {
        speed_emulate_set_target("5150", false);
    }
}

bool speed_emulate_is_active(void) {
    return s_emulate_active;
}

bool speed_emulate_is_io_active(void) {
    return s_emulate_active && s_emulate_io;
}

const char *speed_emulate_get_target(void) {
    return s_emulate_target;
}

double speed_emulate_get_target_hz(void) {
    return s_target_hz;
}

double speed_emulate_get_ratio(void) {
    return s_emulate_ratio;
}

void speed_emulate_pace_statement(void) {
    if (!s_emulate_active || s_stmt_delay_us == 0) return;

    // Batch pacing every 4 statements to reduce timer call overhead
    s_stmt_counter++;
    if ((s_stmt_counter & 0x03) == 0) {
        plat_hw_delay_us(s_stmt_delay_us * 4);
    }
}

void speed_emulate_pace_io(size_t bytes) {
    if (!s_emulate_active || !s_emulate_io || s_io_byte_delay_us == 0 || bytes == 0) return;

    uint64_t total_us = s_io_byte_delay_us * (uint64_t)bytes;
    // Cap single I/O burst delay at 20 milliseconds to prevent freezing UI
    if (total_us > 20000) total_us = 20000;
    plat_hw_delay_us(total_us);
}
