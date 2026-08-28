# `vdev` Virtual Device Subsystem (`libkernel`)

## 1. Architectural Purpose & Overview

The `vdev` subsystem (`engine/src/device/vdev.c`) abstracts character stream I/O, files, console redirection, and hardware peripherals in BASIC++.

### Key Architectural Invariants:
- **Console Redirection Rule**: Never call raw C standard library output functions (`printf`, `putchar`) in statement handlers; route all I/O through `VDevContext`.
- **Channel Multiplexing**: Channels `#0` through `#255` map to files, serial ports, printers (`LPT1:`), and sockets.

---

## 2. Technical API Signatures (C17)

```c
VDevContext *vdev_create(void);
void vdev_destroy(VDevContext *vdev);
int vdev_putc(VDevContext *vdev, int ch);
int vdev_puts(VDevContext *vdev, const char *str);
int vdev_printf(VDevContext *vdev, const char *fmt, ...);
```
