// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel, libserver, libstandard
// NEEDS: libcore (alloc.h, ctype.h, hal.h, memops.h, runtime_snprintf.h, strops.h), libengine (lexer.h), libkernel (vdev.h), libplatform (platform.h)
// Implements virtual device subsystem, registry, subdevice multiplexing, and device aliases.
//
// ---- Includes ----

#include "device/vdev.h"
#include "device/dev_serial.h"
#include "device/dev_atari.h"
#include "device/dev_cpm.h"
#include "device/dev_net_transport.h"
#include "device/dev_pseudo.h"
#include "device/dev_loopback.h"
#include "device/dev_user.h"
#include "device/dev_log.h"
#include "device/dev_msg_bus.h"
#include "device/dev_ipc.h"
#include "device/dev_tape.h"
#include "device/dev_tandy.h"
#include "device/dev_kbd_map.h"
#include "device/dev_framebuffer.h"
#include "device/dev_upnp.h"
#include "platform/platform.h"
#include <stdarg.h>
#include "hardware/speed_emulate.h"
#include "lexer/lexer.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#define MAX_VDEVS 128

struct VDevContext {
    MemoryContext    *mem;
    VDev              devices[MAX_VDEVS];
    size_t            count;
    VDev             *con_cache;
    VDevAliasEntry    aliases[MAX_VDEV_ALIASES];
    size_t            alias_count;
    UserDefinedDevice udds[MAX_VDEV_UDD];
    size_t            udd_count;
};

// Case-insensitive string comparison helper
static int strcmp_nocase(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 ? 1 : (s2 ? -1 : 0);
    while (*s1 && *s2) {
        int c1 = runtime_toupper((unsigned char)*s1);
        int c2 = runtime_toupper((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return runtime_toupper((unsigned char)*s1) - runtime_toupper((unsigned char)*s2);
}

int vdev_parse_subdevice_index(const char *name, char *out_base, size_t base_size) {
    if (!name || !*name) return -1;
    size_t len = runtime_strlen(name);
    if (len < 2 || name[len - 1] != ':') return -1;

    // Check for named user profile or bus/ipc topic, e.g. "USER:guest", "BUS:topic", "IPC:pipe"
    if ((runtime_strncasecmp(name, "USER:", 5) == 0 && len > 5) ||
        (runtime_strncasecmp(name, "USR:", 4) == 0 && len > 4) ||
        (runtime_strncasecmp(name, "BUS:", 4) == 0 && len > 4) ||
        (runtime_strncasecmp(name, "IPC:", 4) == 0 && len > 4)) {
        if (out_base && base_size > 0) {
            runtime_snprintf(out_base, base_size, "%s", name);
        }
        return 100;
    }

    char digit_char = name[len - 2];
    if (runtime_isdigit((unsigned char)digit_char)) {
        int digit = digit_char - '0';
        if (out_base && base_size > 0) {
            size_t base_len = len - 2;
            if (base_len + 2 < base_size) {
                runtime_memcpy(out_base, name, base_len);
                out_base[base_len] = ':';
                out_base[base_len + 1] = '\0';
            }
        }
        return digit;
    }

    // Universal Sub-Device default to channel 1 if unindexed (e.g. CON:, PUN:, CAS:)
    if (out_base && base_size > 0) {
        runtime_snprintf(out_base, base_size, "%s", name);
    }
    return 1;
}

bool vdev_alias_set(VDevContext *ctx, const char *alias_name, const char *target_name) {
    if (!ctx || !alias_name || !target_name || !*alias_name || !*target_name) return false;

    // Check if alias already exists
    for (size_t i = 0; i < ctx->alias_count; ++i) {
        if (ctx->aliases[i].active && strcmp_nocase(ctx->aliases[i].alias, alias_name) == 0) {
            runtime_strncpy(ctx->aliases[i].target, target_name, sizeof(ctx->aliases[i].target) - 1);
            return true;
        }
    }

    if (ctx->alias_count >= MAX_VDEV_ALIASES) return false;
    runtime_strncpy(ctx->aliases[ctx->alias_count].alias, alias_name, sizeof(ctx->aliases[ctx->alias_count].alias) - 1);
    runtime_strncpy(ctx->aliases[ctx->alias_count].target, target_name, sizeof(ctx->aliases[ctx->alias_count].target) - 1);
    ctx->aliases[ctx->alias_count].active = true;
    ctx->alias_count++;
    return true;
}

bool vdev_alias_remove(VDevContext *ctx, const char *alias_name) {
    if (!ctx || !alias_name) return false;
    for (size_t i = 0; i < ctx->alias_count; ++i) {
        if (ctx->aliases[i].active && strcmp_nocase(ctx->aliases[i].alias, alias_name) == 0) {
            ctx->aliases[i].active = false;
            return true;
        }
    }
    return false;
}

const char *vdev_alias_resolve(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return name;
    const char *curr = name;
    int depth = 0;

    while (depth < 8) {
        bool found = false;
        for (size_t i = 0; i < ctx->alias_count; ++i) {
            if (ctx->aliases[i].active && strcmp_nocase(ctx->aliases[i].alias, curr) == 0) {
                curr = ctx->aliases[i].target;
                found = true;
                break;
            }
        }
        if (!found) break;
        depth++;
    }
    return curr;
}

void vdev_alias_list(VDevContext *ctx) {
    if (!ctx) return;
    vdev_printf(ctx, "%-15s -> %s\n", "Alias", "Target Device");
    vdev_printf(ctx, "%-15s -- %s\n", "---------------", "-----------------------------");
    for (size_t i = 0; i < ctx->alias_count; ++i) {
        if (ctx->aliases[i].active) {
            vdev_printf(ctx, "%-15s -> %s\n", ctx->aliases[i].alias, ctx->aliases[i].target);
        }
    }
}

bool vdev_udd_register(VDevContext *ctx, const UserDefinedDevice *udd) {
    if (!ctx || !udd || !udd->name[0]) return false;
    for (size_t i = 0; i < ctx->udd_count; ++i) {
        if (ctx->udds[i].active && strcmp_nocase(ctx->udds[i].name, udd->name) == 0) {
            ctx->udds[i] = *udd;
            return true;
        }
    }
    if (ctx->udd_count >= MAX_VDEV_UDD) return false;
    ctx->udds[ctx->udd_count] = *udd;
    ctx->udds[ctx->udd_count].active = true;
    ctx->udd_count++;
    return true;
}

bool vdev_udd_unregister(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return false;
    for (size_t i = 0; i < ctx->udd_count; ++i) {
        if (ctx->udds[i].active && strcmp_nocase(ctx->udds[i].name, name) == 0) {
            ctx->udds[i].active = false;
            return true;
        }
    }
    return false;
}

UserDefinedDevice *vdev_udd_get(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;
    for (size_t i = 0; i < ctx->udd_count; ++i) {
        if (ctx->udds[i].active && strcmp_nocase(ctx->udds[i].name, name) == 0) {
            return &ctx->udds[i];
        }
    }
    return NULL;
}

static VDevContext s_static_vdev;
static bool s_static_vdev_in_use = false;

VDevContext *vdev_init(MemoryContext *mem) {
    if (!mem) return NULL;
    HalContext *hal = hal_get();
    VDevContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (VDevContext *)hal->mem.alloc(sizeof(VDevContext));
    }
    if (!ctx && !s_static_vdev_in_use) {
        ctx = &s_static_vdev;
        s_static_vdev_in_use = true;
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(VDevContext));
    ctx->mem = mem;
    ctx->count = 0;
    ctx->con_cache = NULL;
    ctx->alias_count = 0;
    ctx->udd_count = 0;

    // Zero built-in aliases invariant: aliases are strictly reserved for the user
    return ctx;
}

void vdev_shutdown(VDevContext *ctx) {
    if (ctx) {
        if (ctx == &s_static_vdev) {
            s_static_vdev_in_use = false;
            return;
        }
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) {
            hal->mem.free(ctx);
        }
    }
}

bool vdev_register(VDevContext *ctx, VDev dev) {
    if (!ctx || ctx->count >= MAX_VDEVS || !dev.name) return false;

    // Check if already registered
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp_nocase(ctx->devices[i].name, dev.name) == 0) {
            ctx->devices[i] = dev;
            return true;
        }
    }

    ctx->devices[ctx->count] = dev;
    if (strcmp_nocase(dev.name, "CON:") == 0 || strcmp_nocase(dev.name, "CON1:") == 0) {
        ctx->con_cache = &ctx->devices[ctx->count];
    }
    ctx->count++;
    return true;
}

bool vdev_unmount(VDevContext *ctx, const char *dev_name) {
    if (!ctx || !dev_name) return false;
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp_nocase(ctx->devices[i].name, dev_name) == 0) {
            if (ctx->devices[i].dev_close) {
                ctx->devices[i].dev_close(&ctx->devices[i]);
            }
            for (size_t j = i; j + 1 < ctx->count; ++j) {
                ctx->devices[j] = ctx->devices[j + 1];
            }
            ctx->count--;
            return true;
        }
    }
    return false;
}

bool vdev_mount(VDevContext *ctx, const char *dev_name, const char *driver_type, const char *options) {
    (void)options;
    if (!ctx || !dev_name || !driver_type) return false;

    VDev new_dev;
    if (strcmp_nocase(driver_type, "SERIAL") == 0 || strcmp_nocase(driver_type, "COM") == 0) {
        new_dev = dev_serial_create(dev_name, 1);
    } else if (strcmp_nocase(driver_type, "ATARI") == 0 || strcmp_nocase(driver_type, "CIO") == 0) {
        new_dev = dev_atari_create(dev_name);
    } else if (strcmp_nocase(driver_type, "CPM") == 0) {
        new_dev = dev_cpm_create(dev_name);
    } else if (strcmp_nocase(driver_type, "NETWORK") == 0 || strcmp_nocase(driver_type, "TCP") == 0 ||
               strcmp_nocase(driver_type, "UDP") == 0 || strcmp_nocase(driver_type, "MAC") == 0) {
        new_dev = dev_net_transport_create(dev_name);
    } else if (strcmp_nocase(driver_type, "LOOPBACK") == 0 || strcmp_nocase(driver_type, "LOOP") == 0) {
        new_dev = dev_loopback_create(dev_name, 1);
    } else if (strcmp_nocase(driver_type, "UPNP") == 0 || strcmp_nocase(driver_type, "NAT") == 0) {
        new_dev = dev_upnp_create(dev_name);
    } else if (strcmp_nocase(driver_type, "USER") == 0 || strcmp_nocase(driver_type, "USR") == 0) {
        new_dev = dev_user_create(dev_name);
    } else if (strcmp_nocase(driver_type, "TAPE") == 0 || strcmp_nocase(driver_type, "CASSETTE") == 0 ||
               strcmp_nocase(driver_type, "CAS") == 0) {
        new_dev = dev_tape_create(dev_name);
    } else if (strcmp_nocase(driver_type, "TANDY") == 0 || strcmp_nocase(driver_type, "TRS80") == 0 ||
               strcmp_nocase(driver_type, "MODEM") == 0 || strcmp_nocase(driver_type, "MDM") == 0) {
        new_dev = dev_tandy_create(dev_name);
    } else if (strcmp_nocase(driver_type, "KEYMAP") == 0 || strcmp_nocase(driver_type, "KBD") == 0) {
        new_dev = dev_kbd_map_create(dev_name, 1);
    } else if (strcmp_nocase(driver_type, "FRAMEBUFFER") == 0 || strcmp_nocase(driver_type, "SCR") == 0) {
        new_dev = dev_framebuffer_create(dev_name, 1);
    } else if (strcmp_nocase(driver_type, "NULL") == 0 || strcmp_nocase(driver_type, "ZERO") == 0 ||
               strcmp_nocase(driver_type, "RANDOM") == 0 || strcmp_nocase(driver_type, "PORT") == 0) {
        new_dev = dev_pseudo_create(dev_name);
    } else {
        new_dev = dev_pseudo_create(dev_name);
    }

    return vdev_register(ctx, new_dev);
}

void vdev_reset_defaults(VDevContext *ctx) {
    if (!ctx) return;
    ctx->alias_count = 0;
    ctx->udd_count = 0;
}

VDev *vdev_get(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;

    // 1. Resolve aliases
    const char *resolved = vdev_alias_resolve(ctx, name);

    // 2. Sub-device indexing rule (DEV1: .. DEV8: allowed, 0 and 9 rejected as reserved)
    char base_name[32];
    int sub_idx = vdev_parse_subdevice_index(resolved, base_name, sizeof(base_name));
    if (sub_idx == 0 || sub_idx == 9) {
        return NULL; // System reserved
    }

    char norm_buf[32];
    const char *canonical = resolved;
    size_t rlen = runtime_strlen(resolved);
    if (rlen >= 2 && resolved[rlen - 1] == ':' && !runtime_isdigit((unsigned char)resolved[rlen - 2]) &&
        runtime_strncasecmp(resolved, "USER:", 5) != 0 && runtime_strncasecmp(resolved, "USR:", 4) != 0 &&
        runtime_strncasecmp(resolved, "BUS:", 4) != 0 && runtime_strncasecmp(resolved, "IPC:", 4) != 0 &&
        runtime_strcasecmp(resolved, "WALL:") != 0 && runtime_strcasecmp(resolved, "LOGGER:") != 0 &&
        runtime_strcasecmp(resolved, "CON:") != 0 && runtime_strcasecmp(resolved, "SCRN:") != 0 &&
        runtime_strcasecmp(resolved, "KYBD:") != 0 && runtime_strcasecmp(resolved, "PRN:") != 0 &&
        runtime_strcasecmp(resolved, "E:") != 0 && runtime_strcasecmp(resolved, "K:") != 0 &&
        runtime_strcasecmp(resolved, "S:") != 0 && runtime_strcasecmp(resolved, "P:") != 0 &&
        runtime_strcasecmp(resolved, "V:") != 0 && runtime_strcasecmp(resolved, "NULL:") != 0 &&
        runtime_strcasecmp(resolved, "ZERO:") != 0 && runtime_strcasecmp(resolved, "PORT:") != 0) {
        // Unindexed sub-device (e.g. "LOOP:", "CAS:", "MDM:") -> normalize to "LOOP1:", "CAS1:", "MDM1:"
        runtime_snprintf(norm_buf, sizeof(norm_buf), "%.*s1:", (int)(rlen - 1), resolved);
        canonical = norm_buf;
    }

    // 3. Check direct registered devices
    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp_nocase(ctx->devices[i].name, canonical) == 0 ||
            strcmp_nocase(ctx->devices[i].name, resolved) == 0) {
            return &ctx->devices[i];
        }
    }

    // 4. On-demand dynamic registration for built-in cross-ecosystem device families
    if (runtime_strncasecmp(canonical, "COM", 3) == 0 || runtime_strncasecmp(canonical, "R", 1) == 0) {
        int pnum = (sub_idx >= 1 && sub_idx <= 8) ? sub_idx : 1;
        VDev sdev = dev_serial_create(canonical, pnum);
        if (vdev_register(ctx, sdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strcasecmp(canonical, "E:") == 0 || runtime_strcasecmp(canonical, "K:") == 0 ||
        runtime_strcasecmp(canonical, "S:") == 0 || runtime_strcasecmp(canonical, "P:") == 0 ||
        runtime_strcasecmp(canonical, "C:") == 0 || runtime_strcasecmp(canonical, "V:") == 0 ||
        runtime_strncasecmp(canonical, "CIO:", 4) == 0) {
        VDev adev = dev_atari_create(canonical);
        if (vdev_register(ctx, adev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "PUN", 3) == 0 || runtime_strncasecmp(canonical, "PTP", 3) == 0 ||
        runtime_strncasecmp(canonical, "RDR", 3) == 0 || runtime_strncasecmp(canonical, "PTR", 3) == 0 ||
        runtime_strncasecmp(canonical, "LST", 3) == 0 || runtime_strncasecmp(canonical, "AXI", 3) == 0 ||
        runtime_strncasecmp(canonical, "AXO", 3) == 0 || runtime_strncasecmp(canonical, "BAT", 3) == 0 ||
        runtime_strncasecmp(canonical, "AUX", 3) == 0) {
        VDev cdev = dev_cpm_create(canonical);
        if (vdev_register(ctx, cdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "NULL", 4) == 0 || runtime_strncasecmp(canonical, "ZERO", 4) == 0 ||
        runtime_strncasecmp(canonical, "RANDOM", 6) == 0 || runtime_strncasecmp(canonical, "URANDOM", 7) == 0 ||
        runtime_strncasecmp(canonical, "PORT", 4) == 0) {
        VDev pdev = dev_pseudo_create(canonical);
        if (vdev_register(ctx, pdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "MAC", 3) == 0 || runtime_strncasecmp(canonical, "TCP", 3) == 0 ||
        runtime_strncasecmp(canonical, "UDP", 3) == 0 || runtime_strncasecmp(canonical, "NET", 3) == 0) {
        VDev ndev = dev_net_transport_create(canonical);
        if (vdev_register(ctx, ndev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "UPNP", 4) == 0 || runtime_strncasecmp(canonical, "NAT", 3) == 0) {
        VDev udev = dev_upnp_create(canonical);
        if (vdev_register(ctx, udev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "LOOP", 4) == 0) {
        int ch = (sub_idx >= 1 && sub_idx <= 8) ? sub_idx : 1;
        VDev ldev = dev_loopback_create(canonical, ch);
        if (vdev_register(ctx, ldev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "USER", 4) == 0 || runtime_strncasecmp(canonical, "USR", 3) == 0 ||
        runtime_strcasecmp(canonical, "WALL:") == 0 || runtime_strcasecmp(canonical, "LOGGER:") == 0) {
        VDev udev = dev_user_create(canonical);
        if (vdev_register(ctx, udev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "LOG", 3) == 0 || runtime_strncasecmp(canonical, "SYSLOG", 6) == 0) {
        VDev gdev = dev_log_create(canonical);
        if (vdev_register(ctx, gdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "BUS", 3) == 0) {
        VDev bdev = dev_msg_bus_create(resolved);
        if (vdev_register(ctx, bdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "IPC", 3) == 0) {
        VDev idev = dev_ipc_create(resolved);
        if (vdev_register(ctx, idev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "CAS", 3) == 0 || runtime_strncasecmp(canonical, "TAP", 3) == 0 ||
        runtime_strncasecmp(canonical, "MAG", 3) == 0) {
        VDev tdev = dev_tape_create(canonical);
        if (vdev_register(ctx, tdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "MDM", 3) == 0 || runtime_strncasecmp(canonical, "TEL", 3) == 0 ||
        runtime_strncasecmp(canonical, "LCD", 3) == 0 || runtime_strncasecmp(canonical, "BAR", 3) == 0 ||
        runtime_strncasecmp(canonical, "WND", 3) == 0 || runtime_strncasecmp(canonical, "LP", 2) == 0 ||
        runtime_strncasecmp(canonical, "FDD", 3) == 0) {
        VDev ydev = dev_tandy_create(canonical);
        if (vdev_register(ctx, ydev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "KBD", 3) == 0 && runtime_strncasecmp(canonical, "KYBD", 4) != 0) {
        int pr = (sub_idx >= 1 && sub_idx <= 8) ? sub_idx : 1;
        VDev kdev = dev_kbd_map_create(canonical, pr);
        if (vdev_register(ctx, kdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "SCR", 3) == 0 && runtime_strncasecmp(canonical, "SCRN", 4) != 0) {
        int idx = (sub_idx >= 1 && sub_idx <= 8) ? sub_idx : 1;
        VDev fdev = dev_framebuffer_create(canonical, idx);
        if (vdev_register(ctx, fdev)) {
            return &ctx->devices[ctx->count - 1];
        }
    }

    if (runtime_strncasecmp(canonical, "CON", 3) == 0 || runtime_strncasecmp(canonical, "TTY", 3) == 0 ||
        runtime_strncasecmp(canonical, "KEY", 3) == 0 || runtime_strncasecmp(canonical, "SCN", 3) == 0 ||
        runtime_strncasecmp(canonical, "KYBD", 4) == 0 || runtime_strncasecmp(canonical, "SCRN", 4) == 0 ||
        runtime_strncasecmp(canonical, "CONS", 4) == 0 || runtime_strncasecmp(canonical, "WORKSTN", 7) == 0 ||
        runtime_strncasecmp(canonical, "WS", 2) == 0 || runtime_strncasecmp(canonical, "5250", 4) == 0 ||
        runtime_strncasecmp(canonical, "TEK", 3) == 0 || runtime_strncasecmp(canonical, "4010", 4) == 0 ||
        runtime_strncasecmp(canonical, "4014", 4) == 0) {
        if (ctx->con_cache) return ctx->con_cache;
    }

    return NULL;
}


int vdev_printf(VDevContext *ctx, const char *fmt, ...) {
    if (!ctx || !fmt) return -1;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int res = runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (res < 0) return -1;

    return vdev_puts(ctx, buf);
}

int vdev_puts(VDevContext *ctx, const char *s) {
    if (!ctx || !s) return -1;
    speed_emulate_pace_io(runtime_strlen(s));
    VDev *con = ctx->con_cache ? ctx->con_cache : vdev_get(ctx, "CON:");
    if (con && con->ops.puts) {
        return con->ops.puts(con, s);
    }
    return -1;
}

int vdev_putc(VDevContext *ctx, int c) {
    if (!ctx) return -1;
    speed_emulate_pace_io(1);
    VDev *con = ctx->con_cache ? ctx->con_cache : vdev_get(ctx, "CON:");
    if (con && con->ops.putc) {
        return con->ops.putc(con, c);
    }
    return -1;
}

char *vdev_gets(VDevContext *ctx, char *buf, size_t size) {
    if (!ctx || !buf || size == 0) return NULL;

    VDev *con = ctx->con_cache ? ctx->con_cache : vdev_get(ctx, "CON:");
    if (con && con->ops.gets) {
        return con->ops.gets(con, buf, size);
    }
    HalContext *hal = hal_get();
    if (!hal) return NULL;
    if (!platform_stdin_is_console() && hal->io.file_read) {
        size_t idx = 0;
        while (idx < size - 1) {
            char c = 0;
            size_t read_bytes = hal->io.file_read(IO_STDIN_HANDLE, &c, 1, 1);
            if (read_bytes == 0) break;
            buf[idx++] = c;
            if (c == '\n') break;
        }
        if (idx == 0) return NULL;
        buf[idx] = '\0';
        return buf;
    }
    if (!hal->io.console_getchar) return NULL;
    size_t idx = 0;
    while (idx < size - 1) {
        int ch = hal->io.console_getchar();
        if (ch == -1 || ch == 3 || ch == 4 || ch == 26) {
            if (idx == 0) return NULL;
            break;
        }
        if (ch == '\r' || ch == '\n') {
            buf[idx++] = '\n';
            break;
        }
        if (ch == '\b' || ch == 127) {
            if (idx > 0) idx--;
            continue;
        }
        if (ch >= 32 && ch < 127) {
            buf[idx++] = (char)ch;
        }
    }
    if (idx == 0) return NULL;
    buf[idx] = '\0';
    return buf;
}


int vdev_read(VDev *d, void *buf, int len) {
    if (!d || !buf) return -1;
    if (d->dev_read) {
        return d->dev_read(d, buf, len);
    }
    return -1;
}

int vdev_write(VDev *d, const void *buf, int len) {
    if (!d || !buf) return -1;
    if (d->dev_write) {
        return d->dev_write(d, buf, len);
    }
    return -1;
}

long vdev_seek(VDev *d, long offset, int whence) {
    if (!d) return -1;
    if (d->dev_seek) {
        return d->dev_seek(d, offset, whence);
    }
    return -1;
}

int vdev_ioctl(VDev *d, int cmd, void *arg) {
    if (!d) return -1;
    if (d->dev_ioctl) {
        return d->dev_ioctl(d, cmd, arg);
    }
    return -1;
}

int vdev_status(VDev *d) {
    if (!d) return -1;
    if (d->dev_status) {
        return d->dev_status(d);
    }
    return 0; // Default ready
}

int vdev_poll(VDev *d) {
    if (!d) return -1;
    if (d->dev_poll) {
        return d->dev_poll(d);
    }
    return 1; // Default data ready
}

const char *vdev_info(VDev *d, const char *key) {
    if (!d || !key) return "";
    if (strcmp_nocase(key, "name") == 0) return d->name;
    if (strcmp_nocase(key, "class") == 0) return vdev_class_name(d->dev_class);
    if (strcmp_nocase(key, "version") == 0) return d->dev_version ? d->dev_version : "1.0";
    if (strcmp_nocase(key, "description") == 0) return d->dev_description ? d->dev_description : "";
    if (d->dev_info) {
        const char *res = d->dev_info(d, key);
        if (res) return res;
    }
    return "";
}

const char *vdev_class_name(VDevClass cls) {
    switch (cls) {
        case VDCLASS_UNKNOWN:     return "Unknown";
        case VDCLASS_CONSOLE:     return "Console";
        case VDCLASS_FILE:        return "File";
        case VDCLASS_SERIAL:      return "Serial";
        case VDCLASS_PRINTER:     return "Printer";
        case VDCLASS_AUDIO:       return "Audio";
        case VDCLASS_NETWORK:     return "Network";
        case VDCLASS_GPIO:        return "GPIO";
        case VDCLASS_I2C:         return "I2C";
        case VDCLASS_SPI:         return "SPI";
        case VDCLASS_SENSOR:      return "Sensor";
        case VDCLASS_DISPLAY:     return "Display";
        case VDCLASS_STORAGE:     return "Storage";
        case VDCLASS_HID:         return "HID";
        case VDCLASS_CAMERA:      return "Camera";
        case VDCLASS_BRIDGE:      return "Bridge";
        case VDCLASS_BLUETOOTH:   return "Bluetooth";
        case VDCLASS_CLIPBOARD:   return "Clipboard";
        case VDCLASS_PIPE:        return "Pipe";
        case VDCLASS_TIMER:       return "Timer";
        case VDCLASS_USER:        return "User";
        case VDCLASS_TAPE:        return "Tape";
        case VDCLASS_MODEM:       return "Modem";
        case VDCLASS_BARCODE:     return "Barcode";
        case VDCLASS_WINDOW:      return "Window";
        case VDCLASS_FRAMEBUFFER: return "Framebuffer";
        case VDCLASS_KEYMAP:      return "Keymap";
        case VDCLASS_LOOPBACK:    return "Loopback";
        case VDCLASS_CUSTOM:      return "Custom";
        default:                  return "Unknown";
    }
}

int vdev_count(VDevContext *ctx) {
    return ctx ? (int)ctx->count : 0;
}

VDev *vdev_get_by_index(VDevContext *ctx, int index) {
    if (!ctx || index < 0 || index >= (int)ctx->count) return NULL;
    return &ctx->devices[index];
}

void vdev_list_all(VDevContext *ctx) {
    if (!ctx) return;
    vdev_printf(ctx, "%-5s %-10s %-12s %-8s %s\n", "Slot", "Name", "Class", "Version", "Description");
    vdev_printf(ctx, "%-5s %-10s %-12s %-8s %s\n", "----", "----", "-----", "-------", "-----------");
    for (size_t i = 0; i < ctx->count; ++i) {
        VDev *d = &ctx->devices[i];
        const char *ver = d->dev_version ? d->dev_version : "1.0";
        const char *desc = d->dev_description ? d->dev_description : "";
        vdev_printf(ctx, "%-5d %-10s %-12s %-8s %s\n",
                    (int)i, d->name, vdev_class_name(d->dev_class), ver, desc);
    }
}

// Global console and graphics state variables for compatibility statements
int g_cursor_x = 0;
int g_cursor_y = 0;
int g_fg_color_idx = 15;
int g_bg_color_idx = 0;
int g_mouse_x = 0;
int g_mouse_y = 0;
int g_mouse_btn = 0;

#if defined(_MSC_VER)
#pragma comment(linker, "/alternatename:vdev_gfx_poll_events=default_vdev_gfx_poll_events")
#pragma comment(linker, "/alternatename:vdev_gfx_force_flush=default_vdev_gfx_force_flush")
#pragma comment(linker, "/alternatename:vdev_gfx_beep=default_vdev_gfx_beep")
#pragma comment(linker, "/alternatename:vdev_sound_free_all=default_vdev_sound_free_all")
#pragma comment(linker, "/alternatename:vdev_image_free_all=default_vdev_image_free_all")
#pragma comment(linker, "/alternatename:vdev_play_sound_freq=default_vdev_play_sound_freq")
#pragma comment(linker, "/alternatename:vdev_music_note_count=default_vdev_music_note_count")
#pragma comment(linker, "/alternatename:vdev_music_queue_length=default_vdev_music_queue_length")
#pragma comment(linker, "/alternatename:stmt_edit_handler=default_stmt_edit_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_screen_handler=default_vdev_legacy_stmt_screen_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_screen_mode_handler=default_vdev_legacy_stmt_screen_mode_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_line_handler=default_vdev_legacy_stmt_line_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_circle_handler=default_vdev_legacy_stmt_circle_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_paint_handler=default_vdev_legacy_stmt_paint_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_preset_handler=default_vdev_legacy_stmt_preset_handler")
#pragma comment(linker, "/alternatename:vdev_legacy_stmt_pset_handler=default_vdev_legacy_stmt_pset_handler")

static void vdev_skip_to_stmt_end(LexerContext *lex) {
    if (!lex) return;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
            break;
        }
        lex_next(lex);
    }
}

void default_vdev_gfx_force_flush(void) {}
void default_vdev_gfx_poll_events(void) {}
void default_vdev_gfx_beep(void) { platform_sound_beep(); }
void default_vdev_sound_free_all(void) {}
void default_vdev_image_free_all(void) {}
void default_vdev_play_sound_freq(double freq, double duration) {
    if (duration <= 0.0) return;
    uint32_t ms = (uint32_t)(duration * 1000.0);
    uint32_t f = (freq > 0.0) ? (uint32_t)freq : 0;
    platform_sound_tone(f, ms);
}
int  default_vdev_music_note_count(void) { return 0; }
int  default_vdev_music_queue_length(void) { return 0; }
BppError default_stmt_edit_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_screen_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_screen_mode_handler(VMContext *vm, int mode) { (void)vm; (void)mode; BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_line_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
BppError default_vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }

#elif defined(__GNUC__) || defined(__clang__)
static void vdev_skip_to_stmt_end(LexerContext *lex) {
    if (!lex) return;
    while (true) {
        BppToken tok = lex_peek(lex);
        if (tok.type == TOK_EOF || tok.type == TOK_EOL || tok.type == TOK_BACKSLASH) {
            break;
        }
        lex_next(lex);
    }
}

__attribute__((weak)) void vdev_gfx_force_flush(void) {}
__attribute__((weak)) void vdev_gfx_poll_events(void) {}
__attribute__((weak)) void vdev_gfx_beep(void) { platform_sound_beep(); }
__attribute__((weak)) void vdev_sound_free_all(void) {}
__attribute__((weak)) void vdev_image_free_all(void) {}
__attribute__((weak)) void vdev_play_sound_freq(double freq, double duration) {
    if (duration <= 0.0) return;
    uint32_t ms = (uint32_t)(duration * 1000.0);
    uint32_t f = (freq > 0.0) ? (uint32_t)freq : 0;
    platform_sound_tone(f, ms);
}
__attribute__((weak)) int  vdev_music_note_count(void) { return 0; }
__attribute__((weak)) int  vdev_music_queue_length(void) { return 0; }
__attribute__((weak)) BppError stmt_edit_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_screen_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_screen_mode_handler(VMContext *vm, int mode) { (void)vm; (void)mode; BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_line_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
__attribute__((weak)) BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex) { (void)vm; vdev_skip_to_stmt_end(lex); BppError err = {0}; return err; }
#endif



