// FILENAME: vdev.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkernel, libserver, libstandard
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for vdev.h.
//
// ---- Includes ----

// FILENAME: vdev.h
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
// NEEDED BY: libkernel (security.c, usb.h, vdev.c, vdev_esp32.h, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for vdev.h.
//
// ---- Includes ----

// FILENAME: vdev.h
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
// NEEDED BY: libkernel (security.c, usb.h, vdev.c, vdev_esp32.h, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for vdev.h.
//
// ---- Includes ----

// FILENAME: vdev.h
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
// NEEDED BY: libkernel (security.c, usb.h, vdev.c, vprinter.h)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for vdev.h.
//
// ---- Includes ----

// FILENAME: vdev.h
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
// NEEDED BY: libkernel (console.c, gfx.c, security.c, vdev.c)
// NEEDED BY: libserver (vnet.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (memory.h)
// NEEDS: libcore (memory.c)
// NEEDS: libkernel (types.h)
// Implements virtual device and graphics rendering logic for vdev.
//
// ---- Includes ----

#ifndef DEVICE_VDEV_H
#define DEVICE_VDEV_H

#include <stddef.h>
#include <stdbool.h>
#include "memory/memory.h"
#include "types/types.h"

typedef struct VDev VDev;

// VDevClass taxonomy (Phase 16)
typedef enum {
    VDCLASS_UNKNOWN = 0,
    VDCLASS_CONSOLE,
    VDCLASS_FILE,
    VDCLASS_SERIAL,
    VDCLASS_PRINTER,
    VDCLASS_AUDIO,
    VDCLASS_NETWORK,
    VDCLASS_GPIO,
    VDCLASS_I2C,
    VDCLASS_SPI,
    VDCLASS_SENSOR,
    VDCLASS_DISPLAY,
    VDCLASS_STORAGE,
    VDCLASS_HID,
    VDCLASS_CAMERA,
    VDCLASS_BRIDGE,
    VDCLASS_BLUETOOTH,
    VDCLASS_CLIPBOARD,
    VDCLASS_PIPE,
    VDCLASS_TIMER,
    VDCLASS_CUSTOM = 99
} VDevClass;

// Per-device capability flags (Phase 16)
#define VDCAP_READ    0x0001
#define VDCAP_WRITE   0x0002
#define VDCAP_BINARY  0x0004
#define VDCAP_SEEK    0x0008
#define VDCAP_ASYNC   0x0010
#define VDCAP_HOTPLUG 0x0020
#define VDCAP_CONTROL 0x0040
#define VDCAP_STATUS  0x0080
#define VDCAP_EVENT   0x0100
#define VDCAP_DUPLEX  0x0200
#define VDCAP_BLOCK   0x0400

#define VDCAP_RW       (VDCAP_READ | VDCAP_WRITE)
#define VDCAP_STREAM   (VDCAP_READ | VDCAP_WRITE | VDCAP_DUPLEX)
#define VDCAP_FILELIKE (VDCAP_READ | VDCAP_WRITE | VDCAP_BINARY | VDCAP_SEEK)

// Standard IOCTL2 control command codes (Phase 16)
#define VDIO_RESET        0
#define VDIO_GET_STATUS   1
#define VDIO_SET_BAUD     2
#define VDIO_SET_PIN      3
#define VDIO_GET_PIN      4
#define VDIO_SET_ADDR     5
#define VDIO_SET_TIMEOUT  6
#define VDIO_GET_ERROR    7
#define VDIO_ENUMERATE    8

#define VDIO_SET_CURSOR   (256 + 0)
#define VDIO_GET_CURSOR   (256 + 1)
#define VDIO_SET_COLOR    (256 + 2)
#define VDIO_GET_SIZE     (256 + 3)
#define VDIO_SET_TITLE    (256 + 4)
#define VDIO_SET_MODE     (256 + 5)
#define VDIO_SCROLL       (256 + 6)
#define VDIO_USER         512

// Table of functional operations for a virtual device
typedef struct {
    int   (*putc)(VDev *dev, int c);
    int   (*puts)(VDev *dev, const char *s);
    int   (*getc)(VDev *dev);
    char *(*gets)(VDev *dev, char *buf, size_t size);
    int   (*flush)(VDev *dev);
    int   (*cls)(VDev *dev);
} VDevOps;

// Concrete Virtual Device Instance
struct VDev {
    const char *name;   // Case-insensitive name, e.g. "CON:", "NULL:"
    VDevOps     ops;    // Operations table
    void       *priv;   // Private instance context pointer

    // Phase 16 VDev2 extensions
    VDevClass   dev_class;
    int         dev_caps;
    const char *dev_version;
    const char *dev_description;
    int         dev_req_caps;   // CAP_ required flags for gating

    // Binary I/O
    int         (*dev_read)(VDev *d, void *buf, int len);
    int         (*dev_write)(VDev *d, const void *buf, int len);
    long        (*dev_seek)(VDev *d, long offset, int whence);

    // Control
    int         (*dev_ioctl)(VDev *d, int cmd, void *arg);

    // Status
    int         (*dev_status)(VDev *d);
    int         (*dev_poll)(VDev *d);
    const char *(*dev_info)(VDev *d, const char *key);

    // Lifecycle hooks
    int         (*dev_open)(VDev *d, const char *path, int mode);
    int         (*dev_close)(VDev *d);
};

// Opaque Virtual Device Context Manager
typedef struct VDevContext VDevContext;

// @brief Initialize the Virtual Device System context.
VDevContext *vdev_init(MemoryContext *mem);

// @brief Shutdown the Virtual Device System and unregister all devices.
void vdev_shutdown(VDevContext *ctx);

// @brief Register a device in the active registry.
// @return true on success, false if table is full or duplicate name.
bool vdev_register(VDevContext *ctx, VDev dev);

// @brief Find a registered virtual device by name.
VDev *vdev_get(VDevContext *ctx, const char *name);

// @brief High-level formatted printing to the virtual console device ("CON:").
int vdev_printf(VDevContext *ctx, const char *fmt, ...);

// @brief Print a string to the virtual console ("CON:").
int vdev_puts(VDevContext *ctx, const char *s);

// @brief Write a single character to the virtual console ("CON:").
int vdev_putc(VDevContext *ctx, int c);

// @brief Read a line of input from the virtual console ("CON:").
char *vdev_gets(VDevContext *ctx, char *buf, size_t size);

// @brief Play standard system beep (800 Hz, 0.25s) or print bell char fallback.
void vdev_play_beep(VDevContext *ctx);

// @brief Get the length of remaining notes in background playback queue.
int vdev_music_queue_length(void);

// @brief Clear all notes in background music playback queue.
void vdev_music_clear(void);
void vdev_play_sound_freq(double freq, double duration_seconds);

// @brief Poll background events (like SDL window close or keyboard queue).
void vdev_gfx_poll_events(void);
void vdev_gfx_force_flush(void);
void vdev_gfx_enable(bool allowed, bool gui_boot);
typedef struct VMContext VMContext;
typedef struct LexerContext LexerContext;

void vdev_gfx_boot_check(VMContext *vm);
int  gfx_get_char_at(int row, int col);
int  gfx_get_attr_at(int row, int col);

BppError vdev_legacy_stmt_screen_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_color_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_line_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_circle_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_pset_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_preset_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_cls_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_paint_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_sound_handler(VMContext *vm, LexerContext *lex);
BppError vdev_legacy_stmt_play_handler(VMContext *vm, LexerContext *lex);

// VDev2 wrappers (Phase 16)
int         vdev_read(VDev *d, void *buf, int len);
int         vdev_write(VDev *d, const void *buf, int len);
long        vdev_seek(VDev *d, long offset, int whence);
int         vdev_ioctl(VDev *d, int cmd, void *arg);
int         vdev_status(VDev *d);
int         vdev_poll(VDev *d);
const char *vdev_info(VDev *d, const char *key);
const char *vdev_class_name(VDevClass cls);
int         vdev_count(VDevContext *ctx);
VDev       *vdev_get_by_index(VDevContext *ctx, int index);
void        vdev_list_all(VDevContext *ctx);

#endif // DEVICE_VDEV_H
