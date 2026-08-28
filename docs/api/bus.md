# C17 API Reference: Virtual Hardware Bus (`device/bus.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual Hardware Bus Subsystem (`device/bus.h`, implemented in `engine/src/device/bus.c`) virtualizes the 16-bit I/O port address space (`0x0000` to `0xFFFF`), real-mode memory-mapped I/O intercept traps, and peripheral hardware emulation for `INP`, `OUT`, `PEEK`, and `POKE` operations in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **16-Bit I/O Port Address Space**: Dispatches hardware port reads and writes across simulated PC peripherals (PIT 8253 timer `0x40-0x43`, PIC 8259 interrupt controller `0x20-0x21`, Keyboard controller `0x60-0x64`, PC speaker `0x61`, CRTC 6845 video controller `0x3D4-0x3D5`).
- **Memory-Mapped I/O Intercept Traps**: Intercepts `PEEK` and `POKE` operations targeting memory-mapped hardware registers (VRAM `0xB8000`, `0xA0000`, BIOS ROM `0xF0000`).
- **Dynamic Port Handler Registration**: Allows modular extensions and emulator drivers to register custom port ranges (`vdev_bus_register_port()`).
- **Multi-System Hardware Profiles**: Emulates I/O bus architectures for IBM PC, PC/XT, PC/AT, PCjr, Tandy 1000, Apple II, Commodore 64, VIC-20, and Atari 800.

## 2. Header Inclusion & Prerequisites

```c
#include "device/bus.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
/* Hardware Bus Model Architecture Profiles */
typedef enum {
    BIOS_MODEL_NONE   = 0,
    BIOS_MODEL_MSDOS  = 1,
    BIOS_MODEL_IBMPC  = 2,
    BIOS_MODEL_PCJR   = 3,
    BIOS_MODEL_PCXT   = 4,
    BIOS_MODEL_PCAT   = 5,
    BIOS_MODEL_APPLE2 = 6,
    BIOS_MODEL_C64    = 7,
    BIOS_MODEL_VIC20  = 8,
    BIOS_MODEL_ATARI  = 9,
    BIOS_MODEL_TANDY  = 10
} MockBiosModel;
```

## 4. Function Prototypes & Operational Contracts

### Port I/O Operations (`INP` / `OUT`)
```c
/**
 * @brief Writes an 8-bit byte to a virtual hardware I/O port (OUT statement).
 * @param port 16-bit I/O port address (0x0000 .. 0xFFFF).
 * @param value Byte value to write (0 .. 255).
 */
void vdev_bus_out(int port, int value);

/**
 * @brief Reads an 8-bit byte from a virtual hardware I/O port (INP function).
 * @param port 16-bit I/O port address (0x0000 .. 0xFFFF).
 * @return 8-bit byte read from the simulated peripheral.
 */
int vdev_bus_in(int port);
```

### Memory-Mapped Intercepts (`PEEK` / `POKE`)
```c
/**
 * @brief Traps read operations at a physical memory address.
 * @param addr 32-bit physical address.
 * @param intercepted Output flag set to true if intercepted by a hardware device.
 * @return Byte value returned by device or RAM.
 */
uint8_t vdev_bus_peek(unsigned long addr, bool *intercepted);

/**
 * @brief Traps write operations at a physical memory address.
 */
void vdev_bus_poke(unsigned long addr, uint8_t value, bool *intercepted);
```

### Port Handler Registration & Model Selection
```c
/**
 * @brief Registers a custom port handler function table for a range of ports.
 * @param start First port in range (inclusive).
 * @param end Last port in range (inclusive).
 * @param read_fn Callback invoked on INP(port).
 * @param write_fn Callback invoked on OUT port, value.
 * @return true on successful registration, false if range overlaps.
 */
bool vdev_bus_register_port(int start, int end, int (*read_fn)(int), void (*write_fn)(int, int));

/**
 * @brief Resets all port mappings to system defaults.
 */
void vdev_bus_reset(void);

/**
 * @brief Sets active hardware bus architecture model.
 */
void vdev_bus_set_model(MockBiosModel model);
MockBiosModel vdev_bus_get_model(void);
```

## 5. Architectural Invariants

- **Sandbox Enforcement**: Direct port access via `INP` and `OUT` requires `CAP_IO` in `SecurityContext`.
- **Pure Software Emulation**: Never issues privileged x86 `in`/`out` machine instructions on host CPUs; all port dispatch is purely virtualized.

## 6. Code Example: Registering a Custom Hardware Peripheral

```c
#include "device/bus.h"
#include <stdio.h>

static int g_custom_reg = 0;

static int custom_read(int port) {
    return g_custom_reg;
}

static void custom_write(int port, int value) {
    g_custom_reg = value & 0xFF;
}

void register_custom_device(void) {
    /* Register device at I/O ports 0x300 - 0x303 */
    vdev_bus_register_port(0x300, 0x303, custom_read, custom_write);
}
```
