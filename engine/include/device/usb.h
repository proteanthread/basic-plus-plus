// FILENAME: usb.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c)
// NEEDED BY: libengine (control.c, data.c, events_internal.h, exec_internal.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Implements virtual device and graphics rendering logic for usb.
//
// ---- Includes ----

#ifndef DEVICE_USB_H
#define DEVICE_USB_H

#include <stdbool.h>
#include <stddef.h>
#include "types/types.h"
#include "device/vdev.h"

#define USB_MAX_PORTS 4

typedef enum {
    USB_DEV_NONE,
    USB_DEV_KEYBOARD,
    USB_DEV_MOUSE,
    USB_DEV_STORAGE,
    USB_DEV_DISPLAY
} BppUsbDevType;

typedef struct {
    BppUsbDevType type;
    char          name[32];
    int           vendor_id;
    int           product_id;
    bool          connected;
} BppUsbDevice;

typedef struct UsbContext UsbContext;

UsbContext *usb_init(MemoryContext *mem);
void        usb_shutdown(UsbContext *ctx);

bool        usb_connect(UsbContext *ctx, int port, BppUsbDevType type, int vid, int pid);
void        usb_disconnect(UsbContext *ctx, int port);
bool        usb_get_port_status(UsbContext *ctx, int port, BppUsbDevice *out_dev);
int         usb_get_connected_count(UsbContext *ctx);

#endif // DEVICE_USB_H
