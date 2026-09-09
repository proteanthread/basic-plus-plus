// FILENAME: speed_db.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libhardware, libengine
// Vintage folklore system speed database implementation.

#include "hardware/speed_db.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

static int case_cmp(const char *s1, const char *s2) {
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

// 20 Canonical IBM Systems from 1981 to 2001
static const LegacySystemSpec s_base_specs[20] = {
    {
        .model = "IBM PC 5150", .year = 1981,
        .cpu_hz = 4772727.0, .cpu_mhz = 4.77, .cpu_arch = "8088",
        .bus_hz = 4772727.0, .bus_mhz = 4.77, .bus_type = "ISA",
        .aliases = {"5150", "PC 5150", "IBM PC", "IBM 5150", "PC"},
        .alias_count = 5
    },
    {
        .model = "IBM PC XT 5160", .year = 1983,
        .cpu_hz = 4772727.0, .cpu_mhz = 4.77, .cpu_arch = "8088",
        .bus_hz = 4772727.0, .bus_mhz = 4.77, .bus_type = "ISA",
        .aliases = {"5160", "XT", "PC XT", "IBM XT", "PC/XT"},
        .alias_count = 5
    },
    {
        .model = "IBM PC AT 5170", .year = 1984,
        .cpu_hz = 6000000.0, .cpu_mhz = 6.00, .cpu_arch = "80286",
        .bus_hz = 6000000.0, .bus_mhz = 6.00, .bus_type = "ISA",
        .aliases = {"5170", "AT", "PC AT", "IBM AT", "PC/AT"},
        .alias_count = 5
    },
    {
        .model = "Later PC AT models", .year = 1985,
        .cpu_hz = 8000000.0, .cpu_mhz = 8.00, .cpu_arch = "80286",
        .bus_hz = 8000000.0, .bus_mhz = 8.00, .bus_type = "ISA",
        .aliases = {"AT-8", "AT-10", "AT-12", "LATER AT", "PC AT 8MHZ"},
        .alias_count = 5
    },
    {
        .model = "IBM PC/XT 286", .year = 1986,
        .cpu_hz = 6000000.0, .cpu_mhz = 6.00, .cpu_arch = "80286",
        .bus_hz = 6000000.0, .bus_mhz = 6.00, .bus_type = "ISA",
        .aliases = {"XT 286", "XT286", "PC/XT 286", "5162"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 25", .year = 1987,
        .cpu_hz = 8000000.0, .cpu_mhz = 8.00, .cpu_arch = "8086",
        .bus_hz = 8000000.0, .bus_mhz = 8.00, .bus_type = "ISA",
        .aliases = {"PS/2 25", "PS2-25", "MODEL 25", "PS2 25"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 30", .year = 1987,
        .cpu_hz = 8000000.0, .cpu_mhz = 8.00, .cpu_arch = "8086",
        .bus_hz = 8000000.0, .bus_mhz = 8.00, .bus_type = "ISA",
        .aliases = {"PS/2 30", "PS2-30", "MODEL 30", "PS2 30"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 50", .year = 1987,
        .cpu_hz = 10000000.0, .cpu_mhz = 10.00, .cpu_arch = "80286",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 50", "PS2-50", "MODEL 50", "PS2 50"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 60", .year = 1987,
        .cpu_hz = 10000000.0, .cpu_mhz = 10.00, .cpu_arch = "80286",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 60", "PS2-60", "MODEL 60", "PS2 60"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 80", .year = 1987,
        .cpu_hz = 20000000.0, .cpu_mhz = 20.00, .cpu_arch = "80386DX",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 80", "PS2-80", "MODEL 80", "PS2 80"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 55 SX", .year = 1989,
        .cpu_hz = 16000000.0, .cpu_mhz = 16.00, .cpu_arch = "80386SX",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 55SX", "PS2-55SX", "MODEL 55SX", "55SX"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 56/57 SX", .year = 1990,
        .cpu_hz = 20000000.0, .cpu_mhz = 20.00, .cpu_arch = "80386SX",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 56SX", "PS/2 57SX", "MODEL 56SX", "56SX", "57SX"},
        .alias_count = 5
    },
    {
        .model = "PS/2 Model 70/80", .year = 1987,
        .cpu_hz = 33000000.0, .cpu_mhz = 33.00, .cpu_arch = "80386",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 70", "PS2-70", "MODEL 70", "PS2 70"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 90", .year = 1992,
        .cpu_hz = 50000000.0, .cpu_mhz = 50.00, .cpu_arch = "80486",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 90", "PS2-90", "MODEL 90", "PS2 90"},
        .alias_count = 4
    },
    {
        .model = "PS/2 Model 95", .year = 1992,
        .cpu_hz = 90000000.0, .cpu_mhz = 90.00, .cpu_arch = "Pentium",
        .bus_hz = 10000000.0, .bus_mhz = 10.00, .bus_type = "MCA",
        .aliases = {"PS/2 95", "PS2-95", "MODEL 95", "PS2 95"},
        .alias_count = 4
    },
    {
        .model = "Aptiva early models", .year = 1994,
        .cpu_hz = 33000000.0, .cpu_mhz = 33.00, .cpu_arch = "80486",
        .bus_hz = 33000000.0, .bus_mhz = 33.00, .bus_type = "Local Bus",
        .aliases = {"APTIVA", "APTIVA EARLY", "APTIVA 486"},
        .alias_count = 3
    },
    {
        .model = "Aptiva M/A/S series", .year = 1995,
        .cpu_hz = 100000000.0, .cpu_mhz = 100.00, .cpu_arch = "Pentium",
        .bus_hz = 66000000.0, .bus_mhz = 66.00, .bus_type = "Motherboard",
        .aliases = {"APTIVA M", "APTIVA A", "APTIVA S", "APTIVA MAS"},
        .alias_count = 4
    },
    {
        .model = "Aptiva C/E/L series", .year = 1996,
        .cpu_hz = 200000000.0, .cpu_mhz = 200.00, .cpu_arch = "Pentium MMX",
        .bus_hz = 66000000.0, .bus_mhz = 66.00, .bus_type = "Motherboard",
        .aliases = {"APTIVA C", "APTIVA E", "APTIVA L", "APTIVA CEL"},
        .alias_count = 4
    },
    {
        .model = "Later Aptiva models", .year = 1998,
        .cpu_hz = 400000000.0, .cpu_mhz = 400.00, .cpu_arch = "Pentium II",
        .bus_hz = 100000000.0, .bus_mhz = 100.00, .bus_type = "Motherboard",
        .aliases = {"APTIVA LATER", "APTIVA P2", "APTIVA 400"},
        .alias_count = 3
    },
    {
        .model = "Aptiva 2178/2198", .year = 1999,
        .cpu_hz = 800000000.0, .cpu_mhz = 800.00, .cpu_arch = "Pentium III",
        .bus_hz = 133000000.0, .bus_mhz = 133.00, .bus_type = "Motherboard",
        .aliases = {"APTIVA 2178", "APTIVA 2198", "APTIVA P3", "APTIVA 800"},
        .alias_count = 4
    }
};

static LegacySystemSpec s_custom_specs[SPEED_DB_MAX_CUSTOM];
static int s_custom_count = 0;

void speed_db_init(void) {
    s_custom_count = 0;
    memset(s_custom_specs, 0, sizeof(s_custom_specs));
}

const LegacySystemSpec *speed_db_find(const char *name) {
    if (!name || !*name) return NULL;

    // First check custom systems (so user overrides take precedence)
    for (int i = 0; i < s_custom_count; i++) {
        if (case_cmp(name, s_custom_specs[i].model) == 0) {
            return &s_custom_specs[i];
        }
        for (int a = 0; a < s_custom_specs[i].alias_count; a++) {
            if (case_cmp(name, s_custom_specs[i].aliases[a]) == 0) {
                return &s_custom_specs[i];
            }
        }
    }

    // Check static base systems
    for (int i = 0; i < 20; i++) {
        if (case_cmp(name, s_base_specs[i].model) == 0) {
            return &s_base_specs[i];
        }
        for (int a = 0; a < s_base_specs[i].alias_count; a++) {
            if (case_cmp(name, s_base_specs[i].aliases[a]) == 0) {
                return &s_base_specs[i];
            }
        }
    }
    return NULL;
}

const LegacySystemSpec *speed_db_find_by_mhz(double mhz) {
    if (mhz <= 0.0) return NULL;
    for (int i = 0; i < s_custom_count; i++) {
        if (fabs(s_custom_specs[i].cpu_mhz - mhz) < 0.05) {
            return &s_custom_specs[i];
        }
    }
    for (int i = 0; i < 20; i++) {
        if (fabs(s_base_specs[i].cpu_mhz - mhz) < 0.05) {
            return &s_base_specs[i];
        }
    }
    return NULL;
}

int speed_db_count(void) {
    return 20 + s_custom_count;
}

const LegacySystemSpec *speed_db_get_at(int index) {
    if (index < 0) return NULL;
    if (index < 20) return &s_base_specs[index];
    int custom_idx = index - 20;
    if (custom_idx < s_custom_count) {
        return &s_custom_specs[custom_idx];
    }
    return NULL;
}

bool speed_db_add_custom(const LegacySystemSpec *spec) {
    if (!spec || !spec->model[0]) return false;
    // Check if already exists in custom list
    for (int i = 0; i < s_custom_count; i++) {
        if (case_cmp(spec->model, s_custom_specs[i].model) == 0) {
            s_custom_specs[i] = *spec;
            return true;
        }
    }
    if (s_custom_count >= SPEED_DB_MAX_CUSTOM) {
        return false;
    }
    s_custom_specs[s_custom_count++] = *spec;
    return true;
}

void speed_db_format_unit(double hz, char *buf, size_t buf_sz) {
    if (!buf || buf_sz == 0) return;
    if (hz >= 1000000000.0) {
        snprintf(buf, buf_sz, "%.2f GHz", hz / 1000000000.0);
    } else if (hz >= 1000000.0) {
        snprintf(buf, buf_sz, "%.2f MHz", hz / 1000000.0);
    } else if (hz >= 1000.0) {
        snprintf(buf, buf_sz, "%.2f kHz", hz / 1000.0);
    } else {
        snprintf(buf, buf_sz, "%.0f Hz", hz);
    }
}

double speed_db_calc_ratio(double host_hz, double target_hz) {
    if (target_hz <= 0.0) return 1.0;
    return host_hz / target_hz;
}
