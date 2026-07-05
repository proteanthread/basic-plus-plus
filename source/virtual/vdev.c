/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vdev.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Virtual Device Layer (VDev), Virtual Machine opcode dispatcher, and platform memory mappings.
 *
 * 2. WHAT TO EXPECT:
 *    VDev slots abstract screen, keyboard, files, and GPIO. VM runs bytecode dispatch.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Virtual memory preset maps, new peripheral classes, custom hardware simulation registers.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    VM opcodes, VM stack math, VDev operations interface.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Trace I/O issues in VDev registration calls. Verify memory offsets are within configured segments.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - vdev.c
 // ---
 //
 // Virtual Device System implementation (VDev2).
 //
 // PURPOSE:
 // Implements the three built-in virtual devices (CON:, ERR:, FILE:)
 // and the device management functions (init, get, register). This
 // is the ONLY file in the interpreter that makes direct calls to
 // C stdio functions for I/O. All other modules route I/O through
 // the VDev function pointers.
 //
 // ADDITIONS:
 // - 64 device slots (up from 16)
 // - Device discovery by name and class
 // - Binary I/O convenience wrappers
 // - IOCTL2, status, poll, info wrappers
 // - Device enumeration (DEVICES command)
 // - Class name lookup
 // - Built-in devices now have class/caps metadata
 //
 // ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "vdev.h"
#include "io/vfs.h"
#include "sdl2_emu.h"
#include "platform.h"

struct GW_Memory;
extern struct GW_Memory *g_gw_mem;

 // Platform-specific headers for beep support.
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <conio.h>
#endif
#include "console.h"
#include "standalone/mock_bios/mock_bios_core.h"

MockBiosContext global_mock_bios = { 0 };
static MockBiosRegs g_vhal_regs = { 0 };

static void vhal_get_registers(void *user_data, uint32_t *ax, uint32_t *bx, uint32_t *cx, uint32_t *dx, uint32_t *flags) {
    (void)user_data;
    *ax = g_vhal_regs.ax;
    *bx = g_vhal_regs.bx;
    *cx = g_vhal_regs.cx;
    *dx = g_vhal_regs.dx;
    *flags = g_vhal_regs.flags;
}

static void vhal_set_registers(void *user_data, uint32_t ax, uint32_t bx, uint32_t cx, uint32_t dx, uint32_t flags) {
    (void)user_data;
    g_vhal_regs.ax = ax;
    g_vhal_regs.bx = bx;
    g_vhal_regs.cx = cx;
    g_vhal_regs.dx = dx;
    g_vhal_regs.flags = flags;
}

static uint8_t vhal_read_mem(void *user_data, uint32_t addr) {
    (void)user_data;
    static uint8_t bda[256];
    if (addr >= 0x0400 && addr < 0x0500) return bda[addr - 0x0400];
    return 0;
}

static void vhal_write_mem(void *user_data, uint32_t addr, uint8_t val) {
    (void)user_data;
    static uint8_t bda[256];
    (void)bda;
    if (addr >= 0x0400 && addr < 0x0500) bda[addr - 0x0400] = val;
}

static void vhal_sleep(void *user_data, int ms) {
    (void)ms;
    (void)user_data;
    // wait for ms, not strictly needed for this mock but good for completeness
}

static int vhal_ioctl_callback(void *user_data, int cmd, void *arg) {
    (void)user_data;
    if (cmd == VDEV_IOCTL_INT10) {
        MockBiosRegs *r = (MockBiosRegs *)arg;
        uint8_t al = r->ax & 0xFF;
        uint8_t ah = (r->ax >> 8) & 0xFF;
        if (ah == 0x0E || ah == 0x09) {
            putchar(al);
        }
        return 1;
    } else if (cmd == VDEV_IOCTL_INT16) {
        MockBiosRegs *r = (MockBiosRegs *)arg;
        r->ax = gw_console_read_char();
        return 1;
    }
    return 0;
}
// --- Device Table ---
 // Static table of all registered devices. Slots 0-2 are reserved
 // for built-in devices. Slots 3-63 are available for user devices.
 //
 // Expanded from 16 to 64 slots. Each VDev is ~200 bytes,
 // so the table is ~12 KB - well within stack/BSS limits on all
 // target platforms including FreeDOS.
static VDev device_table[VDEV_MAX];
static int device_used = 0;

// --- Console Device (CON:) - stdout + stdin ---

static int con_putc(VDev *d, int ch)
{
    (void)d;
    if (global_mock_bios.model != BIOS_MODEL_NONE) {
        g_vhal_regs.ax = 0x0E00 | (ch & 0xFF);
        mock_bios_interrupt(&global_mock_bios, 0x10);
        return 0;
    }
    return (putchar(ch) == EOF) ? -1 : 0;
}

static int con_puts(VDev *d, const char *s)
{
    while (*s) {
        if (con_putc(d, *s++) == -1) return -1;
    }
    return 0;
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
    if (global_mock_bios.model != BIOS_MODEL_NONE) {
        g_vhal_regs.ax = 0x0000;
        mock_bios_interrupt(&global_mock_bios, 0x16);
        return g_vhal_regs.ax & 0xFF;
    }
    return getchar();
}

static int con_gets(VDev *d, char *buf, int max)
{
 (void)d;
 if (fgets(buf, max, stdin) == NULL) return -1;
 return 0;
}

// --- Error Device (ERR:) - stderr ---

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

// --- File Device (FILE:) - fopen/fclose/fprintf/fgets ---

static int file_open(VDev *d, const char *path, const char *mode)
{
 FILE *f;
 char resolved[512];
 int for_write = (strchr(mode, 'w') != NULL || strchr(mode, 'a') != NULL || strchr(mode, '+') != NULL) ? 1 : 0;

 if (vfs_resolve(path, resolved, sizeof(resolved), for_write) != 0) {
  return -1;
 }

 if (d->user_data != NULL) {
 fclose((FILE *)d->user_data);
 d->user_data = NULL;
 }
 f = fopen(resolved, mode);
 if (f == NULL) return -1;
 d->user_data = (void *)f;
 return 0;
}

static int file_close(VDev *d)
{
 if (d->user_data != NULL) {
 fclose((FILE *)d->user_data);
 d->user_data = NULL;
 }
 return 0;
}

static int file_putc(VDev *d, int ch)
{
 if (d->user_data == NULL) return -1;
 return (fputc(ch, (FILE *)d->user_data) == EOF) ? -1 : 0;
}

static int file_puts(VDev *d, const char *s)
{
 if (d->user_data == NULL) return -1;
 return (fputs(s, (FILE *)d->user_data) == EOF) ? -1 : 0;
}

static int file_flush(VDev *d)
{
 if (d->user_data == NULL) return -1;
 fflush((FILE *)d->user_data);
 return 0;
}

static int file_getc(VDev *d)
{
 if (d->user_data == NULL) return -1;
 return fgetc((FILE *)d->user_data);
}

static int file_gets(VDev *d, char *buf, int max)
{
 if (d->user_data == NULL) return -1;
 if (fgets(buf, max, (FILE *)d->user_data) == NULL) return -1;
 return 0;
}

// Binary I/O for FILE: device
static int file_read(VDev *d, void *buf, int len)
{
 if (d->user_data == NULL) return -1;
 return (int)fread(buf, 1, (size_t)len, (FILE *)d->user_data);
}

static int file_write(VDev *d, const void *buf, int len)
{
 if (d->user_data == NULL) return -1;
 return (int)fwrite(buf, 1, (size_t)len, (FILE *)d->user_data);
}

static long file_seek(VDev *d, long offset, int whence)
{
 if (d->user_data == NULL) return -1;
 if (fseek((FILE *)d->user_data, offset, whence) != 0) return -1;
 return ftell((FILE *)d->user_data);
}

// --- NULL: device ---
static int null_putc(VDev *d, int ch)
{
    (void)d;
    (void)ch;
    return 0;
}

static int null_puts(VDev *d, const char *s)
{
    (void)d;
    (void)s;
    return 0;
}

static int null_getc(VDev *d)
{
    (void)d;
    return -1; // EOF
}

static int null_gets(VDev *d, char *buf, int max)
{
    (void)d;
    if (max > 0) buf[0] = '\0';
    return 0;
}

static int null_read(VDev *d, void *buf, int len)
{
    (void)d;
    (void)buf;
    (void)len;
    return 0; // EOF
}

static int null_write(VDev *d, const void *buf, int len)
{
    (void)d;
    (void)buf;
    return len;
}

// --- TIMER: device ---
static int timer_getc(VDev *d)
{
    (void)d;
    return -1;
}

// System time helpers
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

static double mock_time_sec = 0.0;
static int mock_time_active = 0;

void vdev_set_mock_time_active(int active)
{
    mock_time_active = active;
    mock_time_sec = 0.0;
}

void vdev_increment_mock_time(double amt)
{
    mock_time_sec += amt;
}

double vdev_get_time(void)
{
    if (mock_time_active) {
        return mock_time_sec;
    }
#if defined(_WIN32) || defined(_WIN64)
    SYSTEMTIME st;
    GetLocalTime(&st);
    return (double)(st.wHour * 3600 + st.wMinute * 60 + st.wSecond) + (double)st.wMilliseconds / 1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec;
    struct tm *tm = localtime(&t);
    if (tm) {
        return (double)(tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec) + (double)tv.tv_usec / 1000000.0;
    }
    return 0.0;
#endif
}

static int timer_read(VDev *d, void *buf, int len)
{
    (void)d;
    if (len >= (int)sizeof(double)) {
        double t = vdev_get_time();
        memcpy(buf, &t, sizeof(double));
        return (int)sizeof(double);
    }
    return -1;
}

static int timer_ioctl(VDev *d, int cmd, void *arg)
{
    (void)d;
    if (cmd == VDIO_RESET) {
        vdev_set_mock_time_active(0);
        return 0;
    }
    if (cmd == 10) { // Set deterministic/mock mode
        int active = arg ? *(int*)arg : 0;
        vdev_set_mock_time_active(active);
        return 0;
    }
    if (cmd == 11) { // Increment mock time
        double amt = arg ? *(double*)arg : 0.0;
        vdev_increment_mock_time(amt);
        return 0;
    }
    return -1;
}

// --- Device System Functions ---

void vdev_init(void)
{
    // Clear entire device table
    memset(device_table, 0, sizeof(device_table));
    device_used = 0;

    // Initialize Mock BIOS globally
    global_mock_bios.model = BIOS_MODEL_MSDOS;
    global_mock_bios.get_registers = vhal_get_registers;
    global_mock_bios.set_registers = vhal_set_registers;
    global_mock_bios.read_mem = vhal_read_mem;
    global_mock_bios.write_mem = vhal_write_mem;
    global_mock_bios.vdev_sleep = vhal_sleep;
    global_mock_bios.vdev_ioctl = vhal_ioctl_callback;

    // --- CON: device (slot 0) ---
    device_table[VDEV_CON].name = "CON:";
    device_table[VDEV_CON].dev_putc = con_putc;
    device_table[VDEV_CON].dev_puts = con_puts;
    device_table[VDEV_CON].dev_flush = con_flush;
    device_table[VDEV_CON].dev_cls = con_cls;
    device_table[VDEV_CON].dev_getc = con_getc;
    device_table[VDEV_CON].dev_gets = con_gets;
    device_table[VDEV_CON].dev_open = NULL;
    device_table[VDEV_CON].dev_close = NULL;
    device_table[VDEV_CON].user_data = NULL;
    // metadata
    device_table[VDEV_CON].dev_class = VDCLASS_CONSOLE;
    device_table[VDEV_CON].dev_caps = VDCAP_READ | VDCAP_WRITE;
    device_table[VDEV_CON].dev_version = "1.0";
    device_table[VDEV_CON].dev_description = "Console (stdout + stdin)";
    device_table[VDEV_CON].dev_req_caps = 0;

    // --- ERR: device (slot 1) ---
    device_table[VDEV_ERR].name = "ERR:";
    device_table[VDEV_ERR].dev_putc = err_putc;
    device_table[VDEV_ERR].dev_puts = err_puts;
    device_table[VDEV_ERR].dev_flush = err_flush;
    device_table[VDEV_ERR].dev_cls = NULL;
    device_table[VDEV_ERR].dev_getc = NULL;
    device_table[VDEV_ERR].dev_gets = NULL;
    device_table[VDEV_ERR].dev_open = NULL;
    device_table[VDEV_ERR].dev_close = NULL;
    device_table[VDEV_ERR].user_data = NULL;
    // metadata
    device_table[VDEV_ERR].dev_class = VDCLASS_CONSOLE;
    device_table[VDEV_ERR].dev_caps = VDCAP_WRITE;
    device_table[VDEV_ERR].dev_version = "1.0";
    device_table[VDEV_ERR].dev_description = "Error output (stderr)";
    device_table[VDEV_ERR].dev_req_caps = 0;

    // --- FILE: device (slot 2) ---
    device_table[VDEV_FILE].name = "FILE:";
    device_table[VDEV_FILE].dev_putc = file_putc;
    device_table[VDEV_FILE].dev_puts = file_puts;
    device_table[VDEV_FILE].dev_flush = file_flush;
    device_table[VDEV_FILE].dev_cls = NULL;
    device_table[VDEV_FILE].dev_getc = file_getc;
    device_table[VDEV_FILE].dev_gets = file_gets;
    device_table[VDEV_FILE].dev_open = file_open;
    device_table[VDEV_FILE].dev_close = file_close;
    device_table[VDEV_FILE].user_data = NULL;
    // metadata
    device_table[VDEV_FILE].dev_class = VDCLASS_FILE;
    device_table[VDEV_FILE].dev_caps = VDCAP_FILELIKE;
    device_table[VDEV_FILE].dev_version = "1.0";
    device_table[VDEV_FILE].dev_description = "File I/O (stdio)";
    device_table[VDEV_FILE].dev_req_caps = 0;
    // binary I/O
    device_table[VDEV_FILE].dev_read = file_read;
    device_table[VDEV_FILE].dev_write = file_write;
    device_table[VDEV_FILE].dev_seek = file_seek;

    // --- NULL: device (slot 3) ---
    device_table[VDEV_NULL].name = "NULL:";
    device_table[VDEV_NULL].dev_putc = null_putc;
    device_table[VDEV_NULL].dev_puts = null_puts;
    device_table[VDEV_NULL].dev_flush = NULL;
    device_table[VDEV_NULL].dev_cls = NULL;
    device_table[VDEV_NULL].dev_getc = null_getc;
    device_table[VDEV_NULL].dev_gets = null_gets;
    device_table[VDEV_NULL].dev_open = NULL;
    device_table[VDEV_NULL].dev_close = NULL;
    device_table[VDEV_NULL].user_data = NULL;
    // metadata
    device_table[VDEV_NULL].dev_class = VDCLASS_UNKNOWN;
    device_table[VDEV_NULL].dev_caps = VDCAP_READ | VDCAP_WRITE | VDCAP_BINARY;
    device_table[VDEV_NULL].dev_version = "1.0";
    device_table[VDEV_NULL].dev_description = "Null device";
    device_table[VDEV_NULL].dev_req_caps = 0;
    // binary I/O
    device_table[VDEV_NULL].dev_read = null_read;
    device_table[VDEV_NULL].dev_write = null_write;

    // --- TIMER: device (slot 4) ---
    device_table[VDEV_TIMER].name = "TIMER:";
    device_table[VDEV_TIMER].dev_putc = NULL;
    device_table[VDEV_TIMER].dev_puts = NULL;
    device_table[VDEV_TIMER].dev_flush = NULL;
    device_table[VDEV_TIMER].dev_cls = NULL;
    device_table[VDEV_TIMER].dev_getc = timer_getc;
    device_table[VDEV_TIMER].dev_gets = NULL;
    device_table[VDEV_TIMER].dev_open = NULL;
    device_table[VDEV_TIMER].dev_close = NULL;
    device_table[VDEV_TIMER].user_data = NULL;
    // metadata
    device_table[VDEV_TIMER].dev_class = VDCLASS_TIMER;
    device_table[VDEV_TIMER].dev_caps = VDCAP_READ | VDCAP_BINARY | VDCAP_CONTROL;
    device_table[VDEV_TIMER].dev_version = "1.0";
    device_table[VDEV_TIMER].dev_description = "Timer device";
    device_table[VDEV_TIMER].dev_req_caps = 0;
    // binary I/O
    device_table[VDEV_TIMER].dev_read = timer_read;
    device_table[VDEV_TIMER].dev_write = NULL;
    device_table[VDEV_TIMER].dev_ioctl = timer_ioctl;

    device_used = VDEV_USER;
}

VDev *vdev_get(int id)
{
 if (id < 0 || id >= VDEV_MAX) return NULL;
 if (device_table[id].name == NULL) return NULL;
 return &device_table[id];
}

int vdev_register(VDev *dev)
{
 int slot;
 if (dev == NULL) return -1;

 // Find next empty slot
 for (slot = VDEV_USER; slot < VDEV_MAX; slot++) {
 if (device_table[slot].name == NULL) {
 memcpy(&device_table[slot], dev, sizeof(VDev));
 if (slot >= device_used) device_used = slot + 1;
 return slot;
 }
 }
 return -1; // table full
}

// --- Device Discovery ---

 // vdev_find_by_name - Case-insensitive name search.
int vdev_find_by_name(const char *name)
{
 int i;
 if (name == NULL) return -1;

 for (i = 0; i < device_used; i++) {
 const char *dn = device_table[i].name;
 int j, match;
 if (dn == NULL) continue;

 // Case-insensitive compare
 match = 1;
 for (j = 0; ; j++) {
 char a = name[j];
 char b = dn[j];
 if (a >= 'a' && a <= 'z') a = (char)(a - 32);
 if (b >= 'a' && b <= 'z') b = (char)(b - 32);
 if (a != b) { match = 0; break; }
 if (a == '\0') break;
 }
 if (match) return i;
 }
 return -1;
}

 // vdev_find_by_class - Find devices by class.
int vdev_find_by_class(VDevClass cls, int start_from)
{
 int i;
 if (start_from < 0) start_from = 0;
 for (i = start_from; i < device_used; i++) {
 if (device_table[i].name != NULL &&
 device_table[i].dev_class == cls) {
 return i;
 }
 }
 return -1;
}

 // vdev_count_class - Count devices of a given class.
int vdev_count_class(VDevClass cls)
{
 int i, count = 0;
 for (i = 0; i < device_used; i++) {
 if (device_table[i].name != NULL &&
 device_table[i].dev_class == cls) {
 count++;
 }
 }
 return count;
}

 // vdev_class_name - Human-readable class name.
const char *vdev_class_name(VDevClass cls)
{
 switch (cls) {
 case VDCLASS_UNKNOWN: return "Unknown";
 case VDCLASS_CONSOLE: return "Console";
 case VDCLASS_FILE: return "File";
 case VDCLASS_SERIAL: return "Serial";
 case VDCLASS_PRINTER: return "Printer";
 case VDCLASS_AUDIO: return "Audio";
 case VDCLASS_NETWORK: return "Network";
 case VDCLASS_GPIO: return "GPIO";
 case VDCLASS_I2C: return "I2C";
 case VDCLASS_SPI: return "SPI";
 case VDCLASS_SENSOR: return "Sensor";
 case VDCLASS_DISPLAY: return "Display";
 case VDCLASS_STORAGE: return "Storage";
 case VDCLASS_HID: return "HID";
 case VDCLASS_CAMERA: return "Camera";
 case VDCLASS_BRIDGE: return "Bridge";
 case VDCLASS_BLUETOOTH: return "Bluetooth";
 case VDCLASS_CLIPBOARD: return "Clipboard";
 case VDCLASS_PIPE: return "Pipe";
 case VDCLASS_TIMER: return "Timer";
 case VDCLASS_CUSTOM: return "Custom";
 default: return "Unknown";
 }
}

 // vdev_list_all - Print all registered devices.
void vdev_list_all(void)
{
 int i;
 printf("Registered devices:\n");
 printf(" ID Name Class Caps Description\n");
 printf(" -- ---- ----- ---- -----------\n");
 for (i = 0; i < device_used; i++) {
 if (device_table[i].name == NULL) continue;
 printf(" %2d %-12s %-10s %04X %s\n",
 i,
 device_table[i].name,
 vdev_class_name(device_table[i].dev_class),
 device_table[i].dev_caps,
 device_table[i].dev_description ?
 device_table[i].dev_description : "(none)");
 }
}

// --- Convenience I/O Functions (Original) ---

int vdev_putc(VDev *d, int ch)
{
 if (d == NULL || d->dev_putc == NULL) return -1;
 return d->dev_putc(d, ch);
}

int vdev_puts(VDev *d, const char *s)
{
 if (d == NULL || d->dev_puts == NULL) return -1;
 return d->dev_puts(d, s);
}

int vdev_printf(VDev *d, const char *fmt, ...)
{
 char buf[1024];
 va_list ap;

 if (d == NULL || d->dev_puts == NULL) return -1;

 va_start(ap, fmt);
 /* vsnprintf prevents buffer overflow -- always use bounded writes */
 vsnprintf(buf, sizeof(buf), fmt, ap);
 va_end(ap);

 return d->dev_puts(d, buf);
}

int vdev_getc(VDev *d)
{
 if (d == NULL || d->dev_getc == NULL) return -1;
 return d->dev_getc(d);
}

int vdev_gets(VDev *d, char *buf, int max)
{
 if (d == NULL || d->dev_gets == NULL) return -1;
 return d->dev_gets(d, buf, max);
}

int vdev_flush(VDev *d)
{
 if (d == NULL || d->dev_flush == NULL) return -1;
 return d->dev_flush(d);
}

int vdev_cls(VDev *d)
{
 if (d == NULL || d->dev_cls == NULL) return -1;
 return d->dev_cls(d);
}

// --- Binary I/O Convenience ---

int vdev_read(VDev *d, void *buf, int len)
{
 if (d == NULL || d->dev_read == NULL) return -1;
 return d->dev_read(d, buf, len);
}

int vdev_write(VDev *d, const void *buf, int len)
{
 if (d == NULL || d->dev_write == NULL) return -1;
 return d->dev_write(d, buf, len);
}

long vdev_seek(VDev *d, long offset, int whence)
{
 if (d == NULL || d->dev_seek == NULL) return -1;
 return d->dev_seek(d, offset, whence);
}

// --- Control & Status Convenience ---

int vdev_ioctl(VDev *d, int cmd, void *arg)
{
 if (d == NULL || d->dev_ioctl == NULL) return -1;
 return d->dev_ioctl(d, cmd, arg);
}

int vdev_status(VDev *d)
{
 if (d == NULL || d->dev_status == NULL) return 0; // assume ready
 return d->dev_status(d);
}

int vdev_poll(VDev *d)
{
 if (d == NULL || d->dev_poll == NULL) return 0; // no data
 return d->dev_poll(d);
}

const char *vdev_info(VDev *d, const char *key)
{
 if (d == NULL) return NULL;

 // Built-in keys handled here
 if (key != NULL) {
 if (strcmp(key, "name") == 0) return d->name;
 if (strcmp(key, "class") == 0) return vdev_class_name(d->dev_class);
 if (strcmp(key, "version") == 0) return d->dev_version;
 if (strcmp(key, "description") == 0) return d->dev_description;
 }

 // Delegate to device-specific info handler
 if (d->dev_info != NULL) return d->dev_info(d, key);
 return NULL;
}

// --- Platform Convenience Functions (Original - unchanged) ---

void vdev_beep(void)
{
#ifndef NO_SDL2
    if (gw_sdl2_is_active()) {
        gw_sdl2_beep();
        return;
    }
#endif
#if defined(_WIN32) || defined(_WIN64)
 Beep(800, 200);
#else
 putchar('\a');
 fflush(stdout);
#endif
}

void vdev_sound(int freq_hz, int duration_ms)
{
#ifndef NO_SDL2
    if (gw_sdl2_is_active()) {
        gw_sdl2_play_tone((float)freq_hz, duration_ms, 1);
        return;
    }
#endif
 if (freq_hz < 37) freq_hz = 37;
 if (freq_hz > 32767) freq_hz = 32767;
 if (duration_ms < 1) duration_ms = 1;
 if (duration_ms > 30000) duration_ms = 30000;

#if defined(_WIN32) || defined(_WIN64)
 Beep((DWORD)freq_hz, (DWORD)duration_ms);
#else
  // Non-Windows: emit BEL character for audible
  // feedback. True frequency control requires
  // platform audio APIs (ALSA, PulseAudio) which
  // are outside our stdio-only design.
  //
  // For terminals that support it, we also emit
  // an OSC title flash to indicate the tone
  // parameters, then restore the title.
 printf("\033]0;BEEP %dHz %dms\a", freq_hz,
  duration_ms);
 fflush(stdout);
#endif
}

void vdev_sleep(int duration_ms)
{
 if (duration_ms < 1) return;
 if (duration_ms > 30000) duration_ms = 30000;
#ifndef NO_SDL2
 if (gw_sdl2_is_active()) {
     gw_sdl2_delay(duration_ms);
     return;
 }
#endif
 platform_sleep_ms(duration_ms);
}

int vdev_inkey(void)
{
#ifndef NO_SDL2
    if (gw_sdl2_is_active()) {
        gw_sdl2_poll_events();
        return gw_sdl2_get_key();
    }
#endif
#if defined(_WIN32) || defined(_WIN64)
 if (_kbhit()) {
 return _getch();
 }
#endif
 return 0;
}
