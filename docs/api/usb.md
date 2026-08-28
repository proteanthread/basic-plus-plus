# C17 API Reference: Virtual USB Subsystem (`device/usb.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual USB Subsystem (`device/usb.h`, implemented in `engine/src/device/usb.c`) provides virtual USB host controller emulation, 4-port root hub management (`USB_MAX_PORTS`), dynamic peripheral device hot-plugging, and HID/mass-storage device abstraction for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **4-Port Virtual USB Hub**: Manages up to 4 concurrent virtual USB ports (ports 1 to 4).
- **Supported Device Types (`BppUsbDevType`)**:
  - `USB_DEV_KEYBOARD`: Virtual USB HID keyboard device.
  - `USB_DEV_MOUSE`: Virtual USB HID mouse device.
  - `USB_DEV_STORAGE`: Virtual USB mass-storage disk drive.
  - `USB_DEV_DISPLAY`: Virtual USB secondary display monitor.
- **Dynamic Hot-Plugging**: Connects and disconnects virtual devices at runtime with Vendor ID (`VID`) and Product ID (`PID`) identification (`usb_connect()`, `usb_disconnect()`).
- **Sandboxed Device Capability**: Verifies `CAP_USB` capability in `SecurityContext` prior to connecting or communicating with virtual USB ports.

## 2. Header Inclusion & Prerequisites

```c
#include "device/usb.h"
#include "memory/memory.h"
```

## 3. Data Structures & Types

```c
#define USB_MAX_PORTS 4

/* Device Type Classification */
typedef enum {
    USB_DEV_NONE     = 0,
    USB_DEV_KEYBOARD = 1,
    USB_DEV_MOUSE    = 2,
    USB_DEV_STORAGE  = 3,
    USB_DEV_DISPLAY  = 4
} BppUsbDevType;

/* Virtual USB Device Descriptor */
typedef struct {
    BppUsbDevType type;         /* Device class */
    char          name[32];     /* Device product string */
    int           vendor_id;    /* 16-bit VID */
    int           product_id;   /* 16-bit PID */
    bool          connected;    /* True if currently attached */
} BppUsbDevice;

/* Opaque Handle to USB Subsystem Context */
typedef struct UsbContext UsbContext;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the virtual USB host controller context.
 */
UsbContext *usb_init(MemoryContext *mem);

/**
 * @brief Shuts down the USB controller and disconnects all virtual ports.
 */
void usb_shutdown(UsbContext *ctx);

/**
 * @brief Connects a virtual device to a designated USB port (1..4).
 * @return true on success, false if port invalid or already occupied.
 */
bool usb_connect(UsbContext *ctx, int port, BppUsbDevType type, int vid, int pid);

/**
 * @brief Disconnects a device from a USB port.
 */
void usb_disconnect(UsbContext *ctx, int port);

/**
 * @brief Retrieves the status and descriptor of a specific USB port.
 */
bool usb_get_port_status(UsbContext *ctx, int port, BppUsbDevice *out_dev);

/**
 * @brief Returns total number of currently connected USB devices.
 */
int usb_get_connected_count(UsbContext *ctx);
```

## 5. Architectural Invariants

- **Port Range Enforcement**: Port indices must be between 1 and `USB_MAX_PORTS` (4).
- **Capability Guard**: Direct USB manipulation requires `CAP_USB` in `SecurityContext`.

## 6. Code Example: Connecting a Virtual USB Storage Device

```c
#include "device/usb.h"

void attach_flash_drive(UsbContext *usb) {
    /* Connect virtual flash drive at port 1 (VID=0x0781, PID=0x5583) */
    usb_connect(usb, 1, USB_DEV_STORAGE, 0x0781, 0x5583);
}
```
