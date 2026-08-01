# USB Peripherals Abstraction API Reference

Header File: [`include/bpp_usb.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_usb.h)

## Overview
Implements the abstract USB device controller and serial pipe operations.

## Exposed API Entities
### Structs & Types
- `UsbContext UsbContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `usb_shutdown` | `void` | `UsbContext *ctx` |
| `usb_connect` | `bool` | `UsbContext *ctx, int port, BppUsbDevType type, int vid, int pid` |
| `usb_disconnect` | `void` | `UsbContext *ctx, int port` |
| `usb_get_port_status` | `bool` | `UsbContext *ctx, int port, BppUsbDevice *out_dev` |
| `usb_get_connected_count` | `int` | `UsbContext *ctx` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_usb.h"

void write_usb() {
    usb_write_pipe(0, (const unsigned char *)"DATA", 4);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
