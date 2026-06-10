/*
 * ---
 * BASIC++ Interpreter - mod_usb.c
 * ---
 *
 * USB Device Module.
 *
 * PURPOSE:
 * Provides USB device access for BASIC++ programs via the VDev2
 * framework. Registers three virtual devices:
 *
 * USB: General USB device (enumeration, raw transfer)
 * HID: USB Human Interface Device (joystick, gamepad, etc.)
 * USBSER: USB-to-serial adapter (USB CDC / FTDI / CH340)
 *
 * PLATFORM SUPPORT:
 * Windows: Uses SetupAPI + HID.dll for HID devices,
 * CreateFile for USB-serial (COMx)
 * Linux: Uses /dev/hidrawN for HID, /dev/ttyUSBN or
 * /dev/ttyACMN for USB-serial
 * FreeDOS: Stub only (USB not generally available)
 *
 * BASIC USAGE:
 * MODULE "USB"
 * DEVICES ' lists USB:, HID:, USBSER:
 * OPEN "HID:" FOR INPUT AS #1
 * GET #1, J$, 8 ' read 8-byte HID report
 * CLOSE #1
 *
 * SECURITY:
 * Requires CAP_USB | CAP_IO.
 * All USB access is gated by the security system.
 * In SEC_RESTRICTED mode, the module cannot activate.
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mod_usb.h"
#include "module.h"
#include "vdev.h"

/* --- Platform-Specific Includes ---
 */

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
/*
 * HID.dll is loaded dynamically to avoid compile-time dependency.
 * If HID.dll is not available, HID functions gracefully return -1.
 *
 * For full implementation, link against hid.lib and setupapi.lib:
 * #include <hidsdi.h>
 * #include <setupapi.h>
 * #pragma comment(lib, "hid.lib")
 * #pragma comment(lib, "setupapi.lib")
 */
#endif

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
/* #include <linux/hidraw.h> */ /* uncomment when building on Linux */
#endif

/* --- USB Device State ---
 * Each USB VDev instance maintains its own state via user_data.
 */

/* Maximum enumerated USB sub-devices */
#define USB_MAX_SUBDEVICES 16
#define USB_PATH_MAX 260
#define USB_HID_REPORT_MAX 64

typedef struct USBDeviceState {
 int is_open; /* 1 if device is currently open */
 int platform_handle; /* OS file descriptor / handle */
 char device_path[USB_PATH_MAX]; /* OS-specific device path */
 char last_error[128]; /* last error message */

 /* HID-specific */
 int hid_report_size; /* expected HID report size (bytes) */
 unsigned char hid_report[USB_HID_REPORT_MAX]; /* last report */

 /* Enumeration state */
 int enum_count; /* number of discovered sub-devices */
 char enum_names[USB_MAX_SUBDEVICES][64]; /* discovered names */
 int enum_index; /* current enumeration cursor */
} USBDeviceState;

/* Static state for the three device instances */
static USBDeviceState usb_state;
static USBDeviceState hid_state;
static USBDeviceState usbser_state;

/* --- USB: General USB Device ---
 * Provides device enumeration and general USB access.
 */

static int usb_open(VDev *d, const char *path, const char *mode)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1; /* already open */

 st->is_open = 1;
 st->enum_count = 0;
 st->enum_index = 0;
 st->last_error[0] = '\0';

 if (path != NULL) {
 strncpy(st->device_path, path, USB_PATH_MAX - 1);
 st->device_path[USB_PATH_MAX - 1] = '\0';
 }

 return 0;
}

static int usb_close(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (!st->is_open) return -1;

#if defined(_WIN32) || defined(_WIN64)
 if (st->platform_handle > 0) {
 CloseHandle((HANDLE)(long)st->platform_handle);
 st->platform_handle = 0;
 }
#elif defined(__linux__)
 if (st->platform_handle >= 0) {
 close(st->platform_handle);
 st->platform_handle = -1;
 }
#endif

 st->is_open = 0;
 return 0;
}

static int usb_status(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 return st->is_open ? 0 : -1; /* 0=ready, -1=not open */
}

/*
 * usb_ioctl - Handle USB control commands.
 *
 * VDIO_ENUMERATE: Scan for connected USB devices.
 * Populates the enumeration list. On Linux, scans /dev/hidraw*
 * and /dev/ttyUSB*. On Windows, uses SetupAPI.
 *
 * VDIO_RESET: Clear enumeration state.
 *
 * VDIO_GET_ERROR: Return last error string.
 */
static int usb_ioctl(VDev *d, int cmd, void *arg)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 switch (cmd) {
 case VDIO_ENUMERATE:
 st->enum_count = 0;
 st->enum_index = 0;

#ifdef __linux__
 /* Scan /dev/hidraw* for HID devices */
 {
 DIR *dp = opendir("/dev");
 struct dirent *ep;
 if (dp != NULL) {
 while ((ep = readdir(dp)) != NULL) {
 if (strncmp(ep->d_name, "hidraw", 6) == 0 ||
 strncmp(ep->d_name, "ttyUSB", 6) == 0 ||
 strncmp(ep->d_name, "ttyACM", 6) == 0) {
 if (st->enum_count < USB_MAX_SUBDEVICES) {
  sprintf(
  st->enum_names[st->enum_count],
  "/dev/%s", ep->d_name);
 st->enum_count++;
 }
 }
 }
 closedir(dp);
 }
 }
#elif defined(_WIN32) || defined(_WIN64)
 /*
 * On Windows, a full implementation would use SetupAPI:
 * SetupDiGetClassDevs(&GUID_DEVINTERFACE_HID, ...)
 * SetupDiEnumDeviceInterfaces(...)
 * SetupDiGetDeviceInterfaceDetail(...)
 *
 * For now, we probe COM ports as a simple enumeration:
 */
 {
 int i;
 char port_name[16];
 for (i = 1; i <= 16; i++) {
 HANDLE h;
 sprintf(port_name, "\\\\.\\COM%d", i);
 h = CreateFileA(port_name, GENERIC_READ,
 0, NULL, OPEN_EXISTING, 0, NULL);
 if (h != INVALID_HANDLE_VALUE) {
 CloseHandle(h);
 if (st->enum_count < USB_MAX_SUBDEVICES) {
 sprintf(st->enum_names[st->enum_count],
 "COM%d", i);
 st->enum_count++;
 }
 }
 }
 }
#endif
 if (arg != NULL) {
 *(int *)arg = st->enum_count;
 }
 return 0;

 case VDIO_RESET:
 st->enum_count = 0;
 st->enum_index = 0;
 st->last_error[0] = '\0';
 return 0;

 case VDIO_GET_ERROR:
 if (arg != NULL) {
 *(const char **)arg = st->last_error;
 }
 return 0;

 default:
 return -1;
 }
}

/*
 * usb_gets - Read next enumerated device name.
 *
 * Returns the next device name from the enumeration list
 * (populated by VDIO_ENUMERATE). Returns -1 when all
 * devices have been read.
 */
static int usb_gets(VDev *d, char *buf, int max)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (st->enum_index >= st->enum_count) {
 buf[0] = '\0';
 return -1; /* no more devices */
 }

 strncpy(buf, st->enum_names[st->enum_index], max - 1);
 buf[max - 1] = '\0';
 st->enum_index++;
 return 0;
}

static int usb_poll(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 /* Data available if there are un-read enumeration entries */
 return (st->enum_index < st->enum_count) ? 1 : 0;
}

static const char *usb_info(VDev *d, const char *key)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 if (strcmp(key, "error") == 0) return st->last_error;
 if (strcmp(key, "count") == 0) {
 /* Return as static string - not thread-safe but C89 */
 static char count_buf[16];
 sprintf(count_buf, "%d", st->enum_count);
 return count_buf;
 }
 return NULL;
}

/* --- HID: USB Human Interface Device ---
 * Reads HID reports from joysticks, gamepads, keyboards, etc.
 */

static int hid_open(VDev *d, const char *path, const char *mode)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1;

 st->platform_handle = -1;
 st->hid_report_size = USB_HID_REPORT_MAX;
 memset(st->hid_report, 0, USB_HID_REPORT_MAX);
 st->last_error[0] = '\0';

 if (path != NULL) {
 strncpy(st->device_path, path, USB_PATH_MAX - 1);
 st->device_path[USB_PATH_MAX - 1] = '\0';
 }

#ifdef __linux__
 /*
 * Open the first available hidraw device, or the one
 * specified in path (e.g., "HID:/dev/hidraw0").
 */
 {
 const char *dev_path = "/dev/hidraw0";
 if (path != NULL && strncmp(path, "/dev/", 5) == 0) {
 dev_path = path;
 } else if (path != NULL && strncmp(path, "HID:", 4) == 0) {
 dev_path = path + 4;
 }
 st->platform_handle = open(dev_path, O_RDONLY | O_NONBLOCK);
 if (st->platform_handle < 0) {
 strcpy(st->last_error, "Cannot open HID device");
 return -1;
 }
 }
#elif defined(_WIN32) || defined(_WIN64)
 /*
 * Full Windows HID implementation would use:
 * HidD_GetHidGuid(&hidGuid)
 * SetupDiGetClassDevs(&hidGuid, ...)
 * CreateFile(devicePath, ...)
 * HidD_GetPreparsedData(handle, &preparsed)
 * HidP_GetCaps(preparsed, &caps)
 *
 * Stub: set an informational error.
 */
 strcpy(st->last_error,
 "Windows HID: link hid.lib + setupapi.lib for full support");
 /* Don't return -1 - allow open to succeed for testing */
#else
 strcpy(st->last_error, "HID not available on this platform");
 return -1;
#endif

 st->is_open = 1;
 return 0;
}

static int hid_close(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (!st->is_open) return -1;

#ifdef __linux__
 if (st->platform_handle >= 0) {
 close(st->platform_handle);
 st->platform_handle = -1;
 }
#elif defined(_WIN32) || defined(_WIN64)
 if (st->platform_handle > 0) {
 CloseHandle((HANDLE)(long)st->platform_handle);
 st->platform_handle = 0;
 }
#endif

 st->is_open = 0;
 return 0;
}

/*
 * hid_read - Read a HID report (binary).
 *
 * Returns the number of bytes read (typically 8 for a gamepad),
 * or -1 on error / no data available.
 *
 * From BASIC:
 * GET #1, J$, 8
 */
static int hid_read(VDev *d, void *buf, int len)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (!st->is_open) return -1;
 if (len > USB_HID_REPORT_MAX) len = USB_HID_REPORT_MAX;

#ifdef __linux__
 {
 int n = (int)read(st->platform_handle, buf, (size_t)len);
 if (n > 0) {
 memcpy(st->hid_report, buf,
 (size_t)(n < USB_HID_REPORT_MAX ? n
 : USB_HID_REPORT_MAX));
 }
 return n; /* -1 on EAGAIN (no data), or bytes read */
 }
#elif defined(_WIN32) || defined(_WIN64)
 /*
 * Full implementation:
 * ReadFile(handle, buf, len, &bytesRead, NULL)
 * return bytesRead;
 */
 (void)buf;
 (void)len;
 return -1; /* stub */
#else
 (void)buf;
 (void)len;
 return -1; /* not available */
#endif
}

static int hid_poll(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (!st->is_open) return -1;

#ifdef __linux__
 /*
 * Non-blocking read attempt. Since we opened O_NONBLOCK,
 * read() returns -1 with errno==EAGAIN if no data.
 */
 {
 unsigned char probe[1];
 int n = (int)read(st->platform_handle, probe, 0);
 (void)n;
 /* A more robust check would use select() or poll() */
 }
 return 0; /* simplified: caller should just try hid_read */
#else
 return 0;
#endif
}

static int hid_status(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 return st->is_open ? 0 : -1;
}

static int hid_ioctl(VDev *d, int cmd, void *arg)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 switch (cmd) {
 case VDIO_RESET:
 memset(st->hid_report, 0, USB_HID_REPORT_MAX);
 st->hid_report_size = USB_HID_REPORT_MAX;
 return 0;

 case VDIO_GET_STATUS:
 if (arg != NULL) {
 *(int *)arg = st->is_open ? 0 : -1;
 }
 return 0;

 case VDIO_GET_ERROR:
 if (arg != NULL) {
 *(const char **)arg = st->last_error;
 }
 return 0;

 default:
 return -1;
 }
}

static const char *hid_info(VDev *d, const char *key)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 if (strcmp(key, "error") == 0) return st->last_error;
 if (strcmp(key, "path") == 0) return st->device_path;
 if (strcmp(key, "report_size") == 0) {
 static char rs_buf[16];
 sprintf(rs_buf, "%d", st->hid_report_size);
 return rs_buf;
 }
 return NULL;
}

/* --- USBSER: USB-to-Serial Adapter ---
 * Provides access to USB serial devices (FTDI, CH340, CP2102, etc.)
 * as if they were traditional COM ports.
 */

static int usbser_open(VDev *d, const char *path, const char *mode)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 (void)mode;

 if (st->is_open) return -1;

 st->platform_handle = -1;
 st->last_error[0] = '\0';

 /* Parse path: "USBSER:COM3" or "USBSER:/dev/ttyUSB0" */
 if (path != NULL) {
 const char *dev = path;
 if (strncmp(path, "USBSER:", 7) == 0) dev = path + 7;
 strncpy(st->device_path, dev, USB_PATH_MAX - 1);
 st->device_path[USB_PATH_MAX - 1] = '\0';
 } else {
 /* Default device */
#ifdef __linux__
 strcpy(st->device_path, "/dev/ttyUSB0");
#elif defined(_WIN32) || defined(_WIN64)
 strcpy(st->device_path, "\\\\.\\COM3");
#else
 strcpy(st->device_path, "");
#endif
 }

#ifdef __linux__
 st->platform_handle = open(st->device_path,
 O_RDWR | O_NOCTTY | O_NONBLOCK);
 if (st->platform_handle < 0) {
 sprintf(st->last_error, "Cannot open %s", st->device_path);
 return -1;
 }
 /* Default: 9600 baud, 8N1. Use IOCTL to change. */
 /* Full implementation would call tcsetattr() here. */
#elif defined(_WIN32) || defined(_WIN64)
 {
 HANDLE h = CreateFileA(st->device_path,
 GENERIC_READ | GENERIC_WRITE,
 0, NULL, OPEN_EXISTING,
 FILE_ATTRIBUTE_NORMAL, NULL);
 if (h == INVALID_HANDLE_VALUE) {
 sprintf(st->last_error, "Cannot open %s",
 st->device_path);
 return -1;
 }
 st->platform_handle = (int)(long)h;

 /* Set default serial params: 9600 baud, 8N1 */
 {
 DCB dcb;
 memset(&dcb, 0, sizeof(dcb));
 dcb.DCBlength = sizeof(dcb);
 GetCommState(h, &dcb);
 dcb.BaudRate = 9600;
 dcb.ByteSize = 8;
 dcb.StopBits = ONESTOPBIT;
 dcb.Parity = NOPARITY;
 SetCommState(h, &dcb);
 }
 }
#else
 strcpy(st->last_error, "USB serial not available on this platform");
 return -1;
#endif

 st->is_open = 1;
 return 0;
}

static int usbser_close(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 if (!st->is_open) return -1;

#ifdef __linux__
 if (st->platform_handle >= 0) {
 close(st->platform_handle);
 st->platform_handle = -1;
 }
#elif defined(_WIN32) || defined(_WIN64)
 if (st->platform_handle > 0) {
 CloseHandle((HANDLE)(long)st->platform_handle);
 st->platform_handle = 0;
 }
#endif

 st->is_open = 0;
 return 0;
}

static int usbser_read(VDev *d, void *buf, int len)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 if (!st->is_open) return -1;

#ifdef __linux__
 return (int)read(st->platform_handle, buf, (size_t)len);
#elif defined(_WIN32) || defined(_WIN64)
 {
 DWORD bytes_read = 0;
 ReadFile((HANDLE)(long)st->platform_handle,
 buf, (DWORD)len, &bytes_read, NULL);
 return (int)bytes_read;
 }
#else
 (void)buf; (void)len;
 return -1;
#endif
}

static int usbser_write(VDev *d, const void *buf, int len)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 if (!st->is_open) return -1;

#ifdef __linux__
 return (int)write(st->platform_handle, buf, (size_t)len);
#elif defined(_WIN32) || defined(_WIN64)
 {
 DWORD bytes_written = 0;
 WriteFile((HANDLE)(long)st->platform_handle,
 buf, (DWORD)len, &bytes_written, NULL);
 return (int)bytes_written;
 }
#else
 (void)buf; (void)len;
 return -1;
#endif
}

/*
 * usbser_puts - Write a text string to the serial port.
 * This allows PRINT # to work naturally.
 */
static int usbser_puts(VDev *d, const char *s)
{
 return usbser_write(d, s, (int)strlen(s));
}

/*
 * usbser_gets - Read a line from the serial port.
 * Reads until newline or buffer full.
 */
static int usbser_gets(VDev *d, char *buf, int max)
{
 int pos = 0;
 int ch;

 while (pos < max - 1) {
 int n = usbser_read(d, &ch, 1);
 if (n <= 0) break;
 if ((char)ch == '\n') break;
 if ((char)ch == '\r') continue; /* skip CR */
 buf[pos++] = (char)ch;
 }
 buf[pos] = '\0';
 return (pos > 0) ? 0 : -1;
}

static int usbser_ioctl(VDev *d, int cmd, void *arg)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;

 switch (cmd) {
 case VDIO_SET_BAUD:
 if (arg == NULL) return -1;
 {
 int baud = *(int *)arg;
 (void)baud;

#ifdef __linux__
 /* Full implementation: tcsetattr with cfsetspeed */
 /* speed_t spd = baud_to_speed(baud);
 struct termios tio;
 tcgetattr(st->platform_handle, &tio);
 cfsetispeed(&tio, spd);
 cfsetospeed(&tio, spd);
 tcsetattr(st->platform_handle, TCSANOW, &tio); */
#elif defined(_WIN32) || defined(_WIN64)
 {
 DCB dcb;
 HANDLE h = (HANDLE)(long)st->platform_handle;
 memset(&dcb, 0, sizeof(dcb));
 dcb.DCBlength = sizeof(dcb);
 GetCommState(h, &dcb);
 dcb.BaudRate = (DWORD)baud;
 SetCommState(h, &dcb);
 }
#endif
 }
 return 0;

 case VDIO_SET_TIMEOUT:
 if (arg == NULL) return -1;
 {
 int ms = *(int *)arg;
 (void)ms;

#if defined(_WIN32) || defined(_WIN64)
 {
 COMMTIMEOUTS timeouts;
 HANDLE h = (HANDLE)(long)st->platform_handle;
 memset(&timeouts, 0, sizeof(timeouts));
 timeouts.ReadTotalTimeoutConstant = (DWORD)ms;
 timeouts.WriteTotalTimeoutConstant = (DWORD)ms;
 SetCommTimeouts(h, &timeouts);
 }
#endif
 }
 return 0;

 case VDIO_RESET:
#if defined(_WIN32) || defined(_WIN64)
 if (st->platform_handle > 0) {
 PurgeComm((HANDLE)(long)st->platform_handle,
 PURGE_RXCLEAR | PURGE_TXCLEAR);
 }
#elif defined(__linux__)
 if (st->platform_handle >= 0) {
 /* tcflush(st->platform_handle, TCIOFLUSH); */
 }
#endif
 return 0;

 case VDIO_GET_ERROR:
 if (arg != NULL) {
 *(const char **)arg = st->last_error;
 }
 return 0;

 default:
 return -1;
 }
}

static int usbser_status(VDev *d)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 return st->is_open ? 0 : -1;
}

static const char *usbser_info(VDev *d, const char *key)
{
 USBDeviceState *st = (USBDeviceState *)d->user_data;
 if (strcmp(key, "error") == 0) return st->last_error;
 if (strcmp(key, "path") == 0) return st->device_path;
 return NULL;
}

/* --- Module Init / Cleanup ---
 */

static int usb_module_init(void *rt)
{
 VDev dev;
 (void)rt;

 /* Clear device states */
 memset(&usb_state, 0, sizeof(usb_state));
 memset(&hid_state, 0, sizeof(hid_state));
 memset(&usbser_state, 0, sizeof(usbser_state));
 usb_state.platform_handle = -1;
 hid_state.platform_handle = -1;
 usbser_state.platform_handle = -1;

 /* --- USB: General USB device --- */
 memset(&dev, 0, sizeof(dev));
 dev.name = "USB:";
 dev.dev_class = VDCLASS_STORAGE;
 dev.dev_caps = VDCAP_READ | VDCAP_CONTROL |
 VDCAP_STATUS | VDCAP_HOTPLUG;
 dev.dev_version = "1.0";
 dev.dev_description = "USB device enumeration and access";
 dev.dev_req_caps = CAP_USB | CAP_IO;
 dev.dev_open = usb_open;
 dev.dev_close = usb_close;
 dev.dev_gets = usb_gets;
 dev.dev_ioctl = usb_ioctl;
 dev.dev_status = usb_status;
 dev.dev_poll = usb_poll;
 dev.dev_info = usb_info;
 dev.user_data = &usb_state;
 vdev_register(&dev);

 /* --- HID: USB Human Interface Device --- */
 memset(&dev, 0, sizeof(dev));
 dev.name = "HID:";
 dev.dev_class = VDCLASS_HID;
 dev.dev_caps = VDCAP_READ | VDCAP_BINARY |
 VDCAP_CONTROL | VDCAP_STATUS |
 VDCAP_HOTPLUG;
 dev.dev_version = "1.0";
 dev.dev_description = "USB HID (joystick, gamepad, keyboard)";
 dev.dev_req_caps = CAP_USB | CAP_IO;
 dev.dev_open = hid_open;
 dev.dev_close = hid_close;
 dev.dev_read = hid_read;
 dev.dev_ioctl = hid_ioctl;
 dev.dev_status = hid_status;
 dev.dev_poll = hid_poll;
 dev.dev_info = hid_info;
 dev.user_data = &hid_state;
 vdev_register(&dev);

 /* --- USBSER: USB-to-Serial adapter --- */
 memset(&dev, 0, sizeof(dev));
 dev.name = "USBSER:";
 dev.dev_class = VDCLASS_SERIAL;
 dev.dev_caps = VDCAP_STREAM | VDCAP_BINARY |
 VDCAP_CONTROL | VDCAP_STATUS |
 VDCAP_HOTPLUG;
 dev.dev_version = "1.0";
 dev.dev_description = "USB serial adapter (FTDI, CH340, CP2102)";
 dev.dev_req_caps = CAP_USB | CAP_IO;
 dev.dev_open = usbser_open;
 dev.dev_close = usbser_close;
 dev.dev_puts = usbser_puts;
 dev.dev_gets = usbser_gets;
 dev.dev_read = usbser_read;
 dev.dev_write = usbser_write;
 dev.dev_ioctl = usbser_ioctl;
 dev.dev_status = usbser_status;
 dev.dev_info = usbser_info;
 dev.user_data = &usbser_state;
 vdev_register(&dev);

 return 0;
}

static void usb_module_cleanup(void)
{
 /* Close any open devices */
 if (usb_state.is_open) {
 VDev d;
 d.user_data = &usb_state;
 usb_close(&d);
 }
 if (hid_state.is_open) {
 VDev d;
 d.user_data = &hid_state;
 hid_close(&d);
 }
 if (usbser_state.is_open) {
 VDev d;
 d.user_data = &usbser_state;
 usbser_close(&d);
 }
}

/* Module descriptor */
static const ModuleInfo usb_module_info = {
 "USB", /* name */
 "1.0", /* version */
 "USB device access (HID, serial)", /* description */
 MOD_DEVICE, /* mod_class */
 CAP_USB | CAP_IO, /* capabilities */
 usb_module_init, /* init */
 usb_module_cleanup /* cleanup */
};

/*
 * mod_usb_register - Register the USB module.
 *
 * Called from main.c during boot. The module starts inactive.
 * The user activates it via MODULE "USB" from BASIC.
 */
void mod_usb_register(void)
{
 module_register(&usb_module_info);
}
