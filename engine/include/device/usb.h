/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file usb.h
 * @brief Device component implementation and public API surface for usb.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for usb.h within the device subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file usb.h
 * @brief USB Device Driver Subsystem interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares emulated USB controller hubs and driver registration structures.
 * - Why it exists: Emulates dynamic external keyboard, mouse, or disk hot-plugs.
 * - Why it works this way: It tracks active USB device ports and binds callbacks for
 *   VHAL device classes.
 */

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

#endif /* DEVICE_USB_H */
