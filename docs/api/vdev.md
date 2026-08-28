# C17 API Reference: Master Virtual Device Bus (`device/vdev.h`)

## 1. Subsystem Overview & Responsibilities

The Master Virtual Device Bus Subsystem (`device/vdev.h`, implemented in `engine/src/device/vdev.c`) provides the universal peripheral and stream I/O interface for the BASIC++ v6.5.2 engine, unifying consoles, files, serial ports, printers, audio synthesizers, networks, GPIO pins, sensors, and graphics adapters under a standard object model.

Key architectural responsibilities include:
- **Universal Device Abstraction (`VDev`)**: Standardized function table (`VDevOps`) defining polymorphic operations: `open`, `read`, `write`, `getc`, `putc`, `flush`, `ioctl`, and `close`.
- **Taxonomy Classification (`VDevClass`)**:
  - `VDCLASS_CONSOLE`: Virtual terminal screen and keyboard (`CON:`, `SCRN:`, `KYBD:`).
  - `VDCLASS_FILE`: Virtual Filesystem streams and images.
  - `VDCLASS_SERIAL`: RS-232 / UART communications ports (`COM1:` .. `COM4:`).
  - `VDCLASS_PRINTER`: Line printer devices (`LPT1:`, `PRN:`).
  - `VDCLASS_AUDIO`: Sound chips and synthesizer voices (`AUDIO:`, `AY-3-8910:`).
  - `VDCLASS_NETWORK`: Network socket streams (`N:`, `TCP:`, `UDP:`).
  - `VDCLASS_GPIO` / `VDCLASS_I2C` / `VDCLASS_SPI`: Embedded microcontroller buses.
- **Granular Capability Bitmasks**: Declares supported operations (`VDCAP_READ`, `VDCAP_WRITE`, `VDCAP_BINARY`, `VDCAP_SEEK`, `VDCAP_ASYNC`, `VDCAP_HOTPLUG`, `VDCAP_DUPLEX`).
- **Standardized IOCTL Interface**: Configures baud rates, parity, flow control, buffer sizes, and hardware resets dynamically.

## 2. Header Inclusion & Prerequisites

```c
#include "device/vdev.h"
#include "memory/memory.h"
#include "types/types.h"
```

## 3. Data Structures & Types

```c
typedef struct VDev VDev;

/* Device Operation Table */
typedef struct {
    bool   (*init)(VDev *dev, void *user_data);
    bool   (*open)(VDev *dev, const char *path, int mode);
    size_t (*read)(VDev *dev, void *buf, size_t count);
    size_t (*write)(VDev *dev, const void *buf, size_t count);
    int    (*getc)(VDev *dev);
    int    (*putc)(VDev *dev, int c);
    void   (*flush)(VDev *dev);
    int    (*ioctl)(VDev *dev, int cmd, void *arg);
    void   (*close)(VDev *dev);
    void   (*destroy)(VDev *dev);
} VDevOps;

/* Unified Virtual Device Struct */
struct VDev {
    const char *name;           /* Device prefix name (e.g. "COM1:", "LPT1:") */
    VDevClass   device_class;   /* VDCLASS_SERIAL, VDCLASS_PRINTER, etc. */
    uint32_t    capabilities;   /* Bitmask of VDCAP_* flags */
    VDevOps     ops;            /* Operations table */
    void       *priv_data;      /* Private device state */
    bool        is_open;        /* Open status */
};

/* Opaque Virtual Device Bus Context */
typedef struct VDevContext VDevContext;
```

## 4. Function Prototypes & Operational Contracts

### Bus Management
```c
VDevContext *vdev_bus_init(MemoryContext *mem);
void         vdev_bus_shutdown(VDevContext *ctx);

bool         vdev_bus_attach(VDevContext *ctx, VDev *dev);
bool         vdev_bus_detach(VDevContext *ctx, const char *name);
VDev        *vdev_bus_find(VDevContext *ctx, const char *name);
```

### Device I/O Operations
```c
bool   vdev_open(VDev *dev, const char *path, int mode);
void   vdev_close(VDev *dev);
size_t vdev_read(VDev *dev, void *buf, size_t count);
size_t vdev_write(VDev *dev, const void *buf, size_t count);
int    vdev_getc(VDev *dev);
int    vdev_putc(VDev *dev, int c);
void   vdev_puts(VDev *dev, const char *str);
void   vdev_flush(VDev *dev);
int    vdev_ioctl(VDev *dev, int cmd, void *arg);
```

## 5. Architectural Invariants

- **Console Redirection Rule**: Never call raw standard library output functions (`printf`, `putchar`) directly in statement handlers; route all I/O through `VDevContext` / `vdev_puts()`.

## 6. Code Example: Writing to Virtual Device Bus

```c
#include "device/vdev.h"

void write_to_printer(VDevContext *bus, const char *document) {
    VDev *lpt = vdev_bus_find(bus, "LPT1:");
    if (lpt && vdev_open(lpt, NULL, 0)) {
        vdev_puts(lpt, document);
        vdev_close(lpt);
    }
}
```
