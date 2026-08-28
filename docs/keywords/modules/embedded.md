# `embedded` Freestanding Microcontroller Profile

## 1. Architectural Purpose & Overview

The `embedded` target profile compiles BASIC++ v6.5.2 as a freestanding C17 runtime with zero standard C library (`libc`) dependencies, optimized for microcontrollers (ARM Cortex-M, RISC-V, ESP32, AVR).

### Key Architectural Specifications:
- **Footprint**: Scales to under 8 KB RAM for core VM execution.
- **Freestanding C17**: Uses only `<stdint.h>`, `<stdbool.h>`, and `<stddef.h>`.
- **Tri-Mode HAL**: Direct register mapping via `STATIC_INLINE` HAL functions.
- **Hardware Bindings**: Direct GPIO, I2C, SPI, and UART virtual bus mappings.
- **No OS Dependencies**: Eliminates all OS-level memory, thread, or file requirements.

---

## 2. Configuration Macros

```c
#define FREESTANDING 1
#define BIOS_HAS_RTC 0
#define BIOS_HAS_DISK 0
#define BASIC_NO_LIBC 1
```
