# Mock BIOS & Freestanding HAL Porting Guide

## 1. Architectural Blueprint & Overview

The BASIC++ v6.5.2 BIOS subsystem (`libbios`) and Hardware Abstraction Layer (HAL) provide a freestanding C17 virtual machine layer capable of running on bare-metal microcontrollers, host operating systems, and retro PC emulators without standard C library (`libc`) dependencies.

### Tri-Mode HAL Dispatch Architecture:
1. **`STATIC_INLINE` Mode**: Direct hardware register mapped dispatch for embedded microcontrollers (ARM Cortex-M, RISC-V, ESP32) with sub-8KB RAM footprint.
2. **`PLUGGABLE_STRUCT` Mode**: Dynamic function pointer dispatch table (`BiosHalInterface`) used by host desktop emulators and the BASIC++ VM engine.
3. **`MACRO_OVERRIDE` Mode**: Direct BIOS interrupt vector trapping for FreeDOS, UEFI, and real-mode PC targets.

---

## 2. Porting Steps for New Platforms

### Step 1: Implement the Minimal HAL Interface
To port `libbios` to a new board or target, implement the required platform hooks in `bios_hal.c`:
```c
#include "bios/bios_hal.h"

void bios_hal_putc(char ch) {
    // Write character to UART / serial port
    UART_TX(ch);
}

char bios_hal_getc(void) {
    // Poll UART for character input
    return UART_RX();
}

uint32_t bios_hal_get_ticks_ms(void) {
    // Return millisecond timer tick count
    return SysTick_GetMillis();
}
```

### Step 2: Configure Build Feature Gates
Set compile-time gates in your CMake or Makefile target:
```c
#define FREESTANDING 1
#define BIOS_HAS_VIDEO 1
#define BIOS_HAS_KEYBOARD 1
#define BIOS_HAS_RTC 1
#define BIOS_HAS_DISK 0
```

### Step 3: Link with Core VM
Link your compiled HAL implementation with `libkernel` and `libengine`.
