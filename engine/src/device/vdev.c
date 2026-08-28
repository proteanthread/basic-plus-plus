// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel, libserver, libstandard
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements component functionality for vdev.c.
//
// ---- Includes ----

// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (boot.h, common_internal.h, embedded.c, headless.c)
// NEEDED BY: libboot (mobile.c)
// NEEDED BY: libcore (analyzer.h, analyzer.c, clear.c, error.c, file.h)
// NEEDED BY: libcore (file_internal.h, iot_main.c, keyword_props.c, list.c)
// NEEDED BY: libcore (print_using.c, spec.c, using_engine.c)
// NEEDED BY: libcore (variables_internal.h, vdev_core.c, vfs.h, vfs.c)
// NEEDED BY: libengine (alias.c, ast_eval_stmt.c, ast_internal.h, beep.c)
// NEEDED BY: libengine (bios.c, call.c, category.c, chain.c, check.c, circle.c)
// NEEDED BY: libengine (cls.c, color.c, cont.c, context.c, control.c, create.c)
// NEEDED BY: libengine (data.c, debug.c, do.c, draw.c, echo.c, enum.c)
// NEEDED BY: libengine (events_internal.h, exec_internal.h, exec_interrupt.c)
// NEEDED BY: libengine (exit_loop.c, files.c, for.c, function.c, goodbye.c)
// NEEDED BY: libengine (help.c, host.c, input.c, input_file.c, introspection.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, lock.c, loop.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, merge.c)
// NEEDED BY: libengine (mid_stmt.c, module.c, mouse.c, msgbox.c, mux.c, new.c)
// NEEDED BY: libengine (next.c, on_com.c, on_timer.c, override.c, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, play.c, poke.c)
// NEEDED BY: libengine (print.c, pset.c, randomize.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, renum.c, run.c, save.c, scope.c)
// NEEDED BY: libengine (screen.c, seek.c, selftest.c, session_stmts.c)
// NEEDED BY: libengine (shared.c, sound.c, stmt_home.c, stop.c, sub_internal.h)
// NEEDED BY: libengine (system.c, task.h, task.c, test.c, type.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify.c, version.c, view.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wend.c, while.c, window.c)
// NEEDED BY: libengine (with.c)
// NEEDED BY: libkernel (console.c, fujinet.h, gfx.c, gfx_internal.h)
// NEEDED BY: libkernel (security.c, usb.h, vdev_esp32.h, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (vdev.h)
// NEEDS: libplatform (platform.h)
// Implements component functionality for vdev.c.
//
// ---- Includes ----

// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (boot.h, common_internal.h, embedded.c, headless.c)
// NEEDED BY: libboot (mobile.c)
// NEEDED BY: libcore (analyzer.h, analyzer.c, clear.c, error.c, file.h)
// NEEDED BY: libcore (file_internal.h, iot_main.c, keyword_props.c, list.c)
// NEEDED BY: libcore (print_using.c, spec.c, using_engine.c)
// NEEDED BY: libcore (variables_internal.h, vdev_core.c, vfs.h, vfs.c)
// NEEDED BY: libengine (alias.c, ast_eval_stmt.c, ast_internal.h, beep.c)
// NEEDED BY: libengine (bios.c, call.c, category.c, chain.c, check.c, circle.c)
// NEEDED BY: libengine (cls.c, color.c, cont.c, context.c, control.c, create.c)
// NEEDED BY: libengine (data.c, debug.c, do.c, draw.c, echo.c, enum.c)
// NEEDED BY: libengine (events_internal.h, exec_internal.h, exec_interrupt.c)
// NEEDED BY: libengine (exit_loop.c, files.c, for.c, function.c, goodbye.c)
// NEEDED BY: libengine (help.c, host.c, input.c, input_file.c, introspection.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, lock.c, loop.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, merge.c)
// NEEDED BY: libengine (mid_stmt.c, module.c, mouse.c, msgbox.c, mux.c, new.c)
// NEEDED BY: libengine (next.c, on_com.c, on_timer.c, override.c, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, play.c, poke.c)
// NEEDED BY: libengine (print.c, pset.c, randomize.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, renum.c, run.c, save.c, scope.c)
// NEEDED BY: libengine (screen.c, seek.c, selftest.c, session_stmts.c)
// NEEDED BY: libengine (shared.c, sound.c, stmt_home.c, stop.c, sub_internal.h)
// NEEDED BY: libengine (system.c, task.h, task.c, test.c, type.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify.c, version.c, view.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wend.c, while.c, window.c)
// NEEDED BY: libengine (with.c)
// NEEDED BY: libkernel (console.c, fujinet.h, gfx.c, gfx_internal.h)
// NEEDED BY: libkernel (security.c, usb.h, vdev_esp32.h, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (vdev.h)
// NEEDS: libplatform (platform.h)
// Implements component functionality for vdev.c.
//
// ---- Includes ----

// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (boot.h, common_internal.h, embedded.c, headless.c)
// NEEDED BY: libboot (mobile.c)
// NEEDED BY: libcore (analyzer.h, analyzer.c, clear.c, error.c, file.h)
// NEEDED BY: libcore (file_internal.h, keyword_props.c, list.c, print_using.c)
// NEEDED BY: libcore (spec.c, using_engine.c, variables_internal.h)
// NEEDED BY: libcore (vdev_core.c, vfs.h, vfs.c)
// NEEDED BY: libengine (alias.c, ast_eval_stmt.c, ast_internal.h, beep.c)
// NEEDED BY: libengine (bios.c, call.c, category.c, chain.c, check.c, circle.c)
// NEEDED BY: libengine (cls.c, color.c, cont.c, context.c, control.c, create.c)
// NEEDED BY: libengine (data.c, debug.c, do.c, draw.c, echo.c, enum.c)
// NEEDED BY: libengine (events_internal.h, exec_internal.h, exec_interrupt.c)
// NEEDED BY: libengine (exit_loop.c, files.c, for.c, function.c, goodbye.c)
// NEEDED BY: libengine (help.c, host.c, input.c, input_file.c, introspection.c)
// NEEDED BY: libengine (joystick.c, key.c, keyword.c, kill.c, line.c)
// NEEDED BY: libengine (line_input.c, linput.c, load.c, lock.c, loop.c)
// NEEDED BY: libengine (mat_input.c, mat_internal.h, mat_print.c, merge.c)
// NEEDED BY: libengine (mid_stmt.c, module.c, mouse.c, msgbox.c, mux.c, new.c)
// NEEDED BY: libengine (next.c, on_com.c, on_timer.c, override.c, page.c)
// NEEDED BY: libengine (paint.c, palette.c, pause.c, pen.c, play.c, poke.c)
// NEEDED BY: libengine (print.c, pset.c, randomize.c, reformat.c)
// NEEDED BY: libengine (reformat_internal.h, renum.c, run.c, save.c, scope.c)
// NEEDED BY: libengine (screen.c, seek.c, selftest.c, session_stmts.c)
// NEEDED BY: libengine (shared.c, sound.c, stmt_home.c, stop.c, sub_internal.h)
// NEEDED BY: libengine (system.c, task.h, task.c, test.c, type.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify.c, version.c, view.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wend.c, while.c, window.c)
// NEEDED BY: libengine (with.c)
// NEEDED BY: libkernel (console.c, fujinet.h, gfx.c, gfx_internal.h)
// NEEDED BY: libkernel (security.c, usb.h, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (vdev.h)
// NEEDS: libplatform (platform.h)
// Implements component functionality for vdev.c.
//
// ---- Includes ----

// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (boot.h, common_internal.h)
// NEEDED BY: libboot (embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (analyzer.h, file.h, file_internal.h)
// NEEDED BY: libcore (variables_internal.h, vfs.h)
// NEEDED BY: libcore (analyzer.c, clear.c, error.c, keyword_props.c, list.c)
// NEEDED BY: libcore (print_using.c, spec.c, using_engine.c, vdev_core.c)
// NEEDED BY: libcore (vfs.c)
// NEEDED BY: libengine (ast_internal.h, events_internal.h, exec_internal.h)
// NEEDED BY: libengine (mat_internal.h, reformat_internal.h, sub_internal.h)
// NEEDED BY: libengine (task.h, vm_internal.h)
// NEEDED BY: libengine (alias.c, ast_eval_stmt.c, beep.c, bios.c, call.c)
// NEEDED BY: libengine (category.c, chain.c, check.c, circle.c, cls.c, color.c)
// NEEDED BY: libengine (cont.c, context.c, control.c, create.c, data.c)
// NEEDED BY: libengine (debug.c, do.c, draw.c, echo.c, enum.c)
// NEEDED BY: libengine (exec_interrupt.c, exit_loop.c, files.c, for.c)
// NEEDED BY: libengine (function.c, goodbye.c, help.c, host.c, input.c)
// NEEDED BY: libengine (input_file.c, introspection.c, joystick.c, key.c)
// NEEDED BY: libengine (keyword.c, kill.c, line.c, line_input.c, linput.c)
// NEEDED BY: libengine (load.c, lock.c, loop.c, mat_input.c, mat_print.c)
// NEEDED BY: libengine (merge.c, mid_stmt.c, module.c, mouse.c, msgbox.c)
// NEEDED BY: libengine (mux.c, new.c, next.c, on_com.c, on_timer.c, override.c)
// NEEDED BY: libengine (page.c, paint.c, palette.c, pause.c, pen.c, play.c)
// NEEDED BY: libengine (poke.c, print.c, pset.c, randomize.c, reformat.c)
// NEEDED BY: libengine (renum.c, run.c, save.c, scope.c, screen.c, seek.c)
// NEEDED BY: libengine (selftest.c, session_stmts.c, shared.c, sound.c)
// NEEDED BY: libengine (stmt_home.c, stop.c, system.c, task.c, test.c, type.c)
// NEEDED BY: libengine (vbdos_controls.c, vbdos_filebox.c, vbdos_fn.c)
// NEEDED BY: libengine (vbdos_widgets.c, verify.c, version.c, view.c, voice.c)
// NEEDED BY: libengine (wend.c, while.c, window.c, with.c)
// NEEDED BY: libkernel (fujinet.h, gfx_internal.h, usb.h, vprinter.h)
// NEEDED BY: libkernel (console.c, gfx.c, security.c)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (alloc.h, ctype.h, hal.h, memops.h, snprintf.h, strops.h)
// NEEDS: libcore (alloc.c, ctype.c, memops.c, snprintf.c, strops.c)
// NEEDS: libengine (lexer.h)
// NEEDS: libengine (lexer.c)
// NEEDS: libkernel (vdev.h)
// NEEDS: libplatform (platform.h)
// Implements virtual device and graphics rendering logic for vdev.
//
// ---- Includes ----

#include "device/vdev.h"
#include "platform/platform.h"
#include "lexer/lexer.h"
#include <stdarg.h>
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#define MAX_VDEVS 128

struct VDevContext {
    MemoryContext *mem;
    VDev           devices[MAX_VDEVS];
    size_t         count;
    VDev          *con_cache;
};

// Case-insensitive string comparison helper
static int strcmp_nocase(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = runtime_toupper((unsigned char)*s1);
        int c2 = runtime_toupper((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return runtime_toupper((unsigned char)*s1) - runtime_toupper((unsigned char)*s2);
}

VDevContext *vdev_init(MemoryContext *mem) {
    if (!mem) return NULL;
    HalContext *hal = hal_get();
    VDevContext *ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (VDevContext *)hal->mem.alloc(sizeof(VDevContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(VDevContext));
    ctx->mem = mem;
    ctx->count = 0;
    ctx->con_cache = NULL;
    return ctx;
}

void vdev_shutdown(VDevContext *ctx) {
    if (ctx) {
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) {
            hal->mem.free(ctx);
        }
    }
}


bool vdev_register(VDevContext *ctx, VDev dev) {
    if (!ctx || ctx->count >= MAX_VDEVS || !dev.name) return false;

    // Check if already registered
    if (vdev_get(ctx, dev.name) != NULL) {
        return false;
    }

    ctx->devices[ctx->count] = dev;
    if (strcmp_nocase(dev.name, "CON:") == 0) {
        ctx->con_cache = &ctx->devices[ctx->count];
    }
    ctx->count++;
    return true;
}

VDev *vdev_get(VDevContext *ctx, const char *name) {
    if (!ctx || !name) return NULL;

    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp_nocase(ctx->devices[i].name, name) == 0) {
            return &ctx->devices[i];
        }
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

    VDev *con = ctx->con_cache ? ctx->con_cache : vdev_get(ctx, "CON:");
    if (con && con->ops.puts) {
        return con->ops.puts(con, s);
    }
    return -1;
}

int vdev_putc(VDevContext *ctx, int c) {
    if (!ctx) return -1;

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
        case VDCLASS_UNKNOWN:   return "Unknown";
        case VDCLASS_CONSOLE:   return "Console";
        case VDCLASS_FILE:      return "File";
        case VDCLASS_SERIAL:    return "Serial";
        case VDCLASS_PRINTER:   return "Printer";
        case VDCLASS_AUDIO:     return "Audio";
        case VDCLASS_NETWORK:   return "Network";
        case VDCLASS_GPIO:      return "GPIO";
        case VDCLASS_I2C:       return "I2C";
        case VDCLASS_SPI:       return "SPI";
        case VDCLASS_SENSOR:    return "Sensor";
        case VDCLASS_DISPLAY:   return "Display";
        case VDCLASS_STORAGE:   return "Storage";
        case VDCLASS_HID:       return "HID";
        case VDCLASS_CAMERA:    return "Camera";
        case VDCLASS_BRIDGE:    return "Bridge";
        case VDCLASS_BLUETOOTH: return "Bluetooth";
        case VDCLASS_CLIPBOARD: return "Clipboard";
        case VDCLASS_PIPE:      return "Pipe";
        case VDCLASS_TIMER:     return "Timer";
        case VDCLASS_CUSTOM:    return "Custom";
        default:                return "Unknown";
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



