/**
 * @file vdev_usb.c
 * @brief USB Device Driver Subsystem implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements USB ports connectivity, controller states, and mock
 *   keyboard/mouse/storage mappings.
 * - Why it exists: Emulates PC-style USB hub architecture for hardware connectivity.
 * - Why it works this way: It tracks active ports using an array in context, defaulting
 *   to pre-attached mock keyboard and mouse.
 */

#include "bpp_usb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct UsbContext {
    MemoryContext *mem;
    BppUsbDevice   ports[USB_MAX_PORTS];
};

UsbContext *usb_init(MemoryContext *mem) {
    if (!mem) return NULL;
    UsbContext *ctx = (UsbContext *)malloc(sizeof(UsbContext));
    if (!ctx) return NULL;
    ctx->mem = mem;

    /* Initialize ports to empty */
    for (int i = 0; i < USB_MAX_PORTS; ++i) {
        ctx->ports[i].type = USB_DEV_NONE;
        ctx->ports[i].name[0] = '\0';
        ctx->ports[i].vendor_id = 0;
        ctx->ports[i].product_id = 0;
        ctx->ports[i].connected = false;
    }

    /* Pre-populate Port 0 with Mock Keyboard, Port 1 with Mock Mouse */
    usb_connect(ctx, 0, USB_DEV_KEYBOARD, 0x04F2, 0x0010);
    usb_connect(ctx, 1, USB_DEV_MOUSE, 0x04F2, 0x0020);

    return ctx;
}

void usb_shutdown(UsbContext *ctx) {
    if (ctx) {
        free(ctx);
    }
}

bool usb_connect(UsbContext *ctx, int port, BppUsbDevType type, int vid, int pid) {
    if (!ctx || port < 0 || port >= USB_MAX_PORTS) return false;

    ctx->ports[port].type = type;
    ctx->ports[port].vendor_id = vid;
    ctx->ports[port].product_id = pid;
    ctx->ports[port].connected = true;

    switch (type) {
        case USB_DEV_KEYBOARD:
            snprintf(ctx->ports[port].name, sizeof(ctx->ports[port].name), "USB Standard Keyboard");
            break;
        case USB_DEV_MOUSE:
            snprintf(ctx->ports[port].name, sizeof(ctx->ports[port].name), "USB Optical Mouse");
            break;
        case USB_DEV_STORAGE:
            snprintf(ctx->ports[port].name, sizeof(ctx->ports[port].name), "USB Flash Storage Disk");
            break;
        case USB_DEV_DISPLAY:
            snprintf(ctx->ports[port].name, sizeof(ctx->ports[port].name), "USB External Monitor");
            break;
        default:
            snprintf(ctx->ports[port].name, sizeof(ctx->ports[port].name), "Unknown USB Device");
            break;
    }

    return true;
}

void usb_disconnect(UsbContext *ctx, int port) {
    if (ctx && port >= 0 && port < USB_MAX_PORTS) {
        ctx->ports[port].type = USB_DEV_NONE;
        ctx->ports[port].name[0] = '\0';
        ctx->ports[port].vendor_id = 0;
        ctx->ports[port].product_id = 0;
        ctx->ports[port].connected = false;
    }
}

bool usb_get_port_status(UsbContext *ctx, int port, BppUsbDevice *out_dev) {
    if (!ctx || port < 0 || port >= USB_MAX_PORTS || !out_dev) return false;
    *out_dev = ctx->ports[port];
    return true;
}

int usb_get_connected_count(UsbContext *ctx) {
    if (!ctx) return 0;
    int count = 0;
    for (int i = 0; i < USB_MAX_PORTS; ++i) {
        if (ctx->ports[i].connected) {
            count++;
        }
    }
    return count;
}
