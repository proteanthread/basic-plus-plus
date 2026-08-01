# Virtual Bus API Reference

Header File: [`include/bpp_bus.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_bus.h)

## Overview
Controls abstract device communication bus protocols and device attachment.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vdev_bus_out` | `void` | `int port, int value` |
| `vdev_bus_in` | `int` | `int port` |
| `vdev_bus_peek` | `uint8_t` | `unsigned long addr, bool *intercepted` |
| `vdev_bus_poke` | `void` | `unsigned long addr, uint8_t value, bool *intercepted` |
| `vdev_bus_reset` | `void` | `void` |
| `vdev_bus_set_model` | `void` | `MockBiosModel model` |
| `vdev_bus_get_model` | `MockBiosModel` | `void` |
| `vdev_bus_set_ram` | `void` | `uint8_t *ram, size_t size` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_bus.h"

void attach_bus() {
    bus_attach_device(0, "USB:");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
