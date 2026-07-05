/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vcon.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Virtual Console (CON:) and Error (ERR:) VHAL Device Drivers.
 * ===================================================================== */

#include "vdev.h"
#include "sdl2_emu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- CON: device callbacks ---

static int con_putc(VDev *d, int ch)
{
    (void)d;
    return (putchar(ch) == EOF) ? -1 : 0;
}

static int con_puts(VDev *d, const char *s)
{
    (void)d;
    return (fputs(s, stdout) == EOF) ? -1 : 0;
}

static int con_flush(VDev *d)
{
    (void)d;
    extern int g_screen_lock;
    if (!g_screen_lock) {
        fflush(stdout);
    }
    return 0;
}

static int con_cls(VDev *d)
{
    (void)d;
#ifndef NO_SDL2
    if (gw_sdl2_is_active()) {
        gw_sdl2_clear_screen(gw_sdl2_get_text_bg());
        return 0;
    }
#endif
    printf("\033[2J\033[H");
    fflush(stdout);
    return 0;
}

static int con_getc(VDev *d)
{
    (void)d;
    return getchar();
}

static int con_gets(VDev *d, char *buf, int max)
{
    (void)d;
    if (fgets(buf, max, stdin) == NULL) return -1;
    return 0;
}

// --- ERR: device callbacks ---

static int err_putc(VDev *d, int ch)
{
    (void)d;
    return (fputc(ch, stderr) == EOF) ? -1 : 0;
}

static int err_puts(VDev *d, const char *s)
{
    (void)d;
    return (fputs(s, stderr) == EOF) ? -1 : 0;
}

static int err_flush(VDev *d)
{
    (void)d;
    fflush(stderr);
    return 0;
}

void vcon_init_device(VDev *d)
{
    d->name = "CON:";
    d->dev_putc = con_putc;
    d->dev_puts = con_puts;
    d->dev_flush = con_flush;
    d->dev_cls = con_cls;
    d->dev_getc = con_getc;
    d->dev_gets = con_gets;
    d->dev_open = NULL;
    d->dev_close = NULL;
    d->user_data = NULL;
    d->dev_class = VDCLASS_CONSOLE;
    d->dev_caps = VDCAP_READ | VDCAP_WRITE;
    d->dev_version = "1.0";
    d->dev_description = "Console (stdout + stdin)";
    d->dev_req_caps = 0;
}

void verr_init_device(VDev *d)
{
    d->name = "ERR:";
    d->dev_putc = err_putc;
    d->dev_puts = err_puts;
    d->dev_flush = err_flush;
    d->dev_cls = NULL;
    d->dev_getc = NULL;
    d->dev_gets = NULL;
    d->dev_open = NULL;
    d->dev_close = NULL;
    d->user_data = NULL;
    d->dev_class = VDCLASS_CONSOLE;
    d->dev_caps = VDCAP_WRITE;
    d->dev_version = "1.0";
    d->dev_description = "Error output (stderr)";
    d->dev_req_caps = 0;
}
