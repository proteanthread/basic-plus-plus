# C17 API Reference: PC BIOS Virtualization (`bios/bios.h`)

## 1. Subsystem Overview & Responsibilities

The BIOS Virtualization Subsystem (`bios/bios.h`, implemented in `engine/src/bios/bios.c`) provides freestanding PC/XT/AT/PCjr system firmware virtualization, standard x86 real-mode software interrupts, BIOS Data Area (BDA) state management, and CRTC hardware register simulation for BASIC++ v6.5.2.

Key architectural responsibilities include:
- **Freestanding C17 Implementation**: Implemented with zero libc runtime dependencies, compiling on 8-bit/32-bit microcontrollers, host OS emulators, or bare-metal FreeDOS/UEFI.
- **Accurate Model Classification**: Emulates IBM PC 5150, PC/XT 5160, PC/AT 5170, and PCjr 4860 across 11 historical release revisions.
- **x86 Real-Mode Interrupt Dispatch**: Handles standard PC interrupts including Video (`INT 10h`), Equipment Check (`INT 11h`), Memory Size (`INT 12h`), Disk Services (`INT 13h`), Serial/Comms (`INT 14h`), Cassette/Misc (`INT 15h`), Keyboard Services (`INT 16h`), Printer (`INT 17h`), and Real-Time Clock/Timer (`INT 1Ah`).
- **BDA Video & State Reflection**:authoritative storage for cursor positions (`0x0450`), screen columns (`0x044A`), active display page (`0x0462`), and timer ticks (`0x046C`).
- **Tri-Mode CPU Execution**: Dispatches via high-level emulation (HLE), native CPU callbacks, or the micro-8086 interpreter (`libcpu8086`).

## 2. Header Inclusion & Prerequisites

```c
#include "bios/bios.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
/* Target Hardware Models */
typedef enum {
    BIOS_MODEL_IBM_PC   = 0, /* IBM PC 5150 (Model 0xFF) */
    BIOS_MODEL_IBM_XT   = 1, /* IBM PC/XT 5160 (Model 0xFE) */
    BIOS_MODEL_IBM_AT   = 2, /* IBM PC/AT 5170 (Model 0xFC) */
    BIOS_MODEL_IBM_PCJR = 3  /* IBM PCjr 4860 (Model 0xFD) */
} BiosModel;

/* x86 16-Bit Register Set */
typedef struct {
    union { struct { uint8_t al, ah; }; uint16_t ax; };
    union { struct { uint8_t bl, bh; }; uint16_t bx; };
    union { struct { uint8_t cl, ch; }; uint16_t cx; };
    union { struct { uint8_t dl, dh; }; uint16_t dx; };
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t sp;
    uint16_t ds;
    uint16_t es;
    uint16_t flags;
} BiosRegisters;

/* Opaque BIOS Context */
typedef struct BiosContext BiosContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle & Configuration
```c
/**
 * @brief Initializes the BIOS virtualization subsystem for a specific PC model.
 */
BiosContext *bios_init(BiosModel model);

/**
 * @brief Shuts down the BIOS context and releases simulated BDA memory.
 */
void bios_shutdown(BiosContext *ctx);

/**
 * @brief Resets BIOS registers, BDA vectors, and timer tick counters to boot defaults.
 */
void bios_reset(BiosContext *ctx);
```

### Interrupt Invocation & BDA Access
```c
/**
 * @brief Dispatches a simulated x86 software interrupt (e.g. 0x10, 0x16, 0x1A).
 * @param ctx BIOS context.
 * @param int_num Interrupt number (0x10 .. 0x1A).
 * @param regs Input/output pointer to x86 registers.
 * @return true if handled, false if interrupt unsupported.
 */
bool bios_interrupt(BiosContext *ctx, uint8_t int_num, BiosRegisters *regs);

/**
 * @brief Retrieves a direct pointer to the 256-byte BIOS Data Area (0x0040:0x0000).
 */
uint8_t *bios_get_bda(BiosContext *ctx);

/**
 * @brief Updates timer tick count at 18.2065 Hz (INT 1Ah / BDA 0x046C).
 */
void bios_timer_tick(BiosContext *ctx);
```

## 5. Architectural Invariants

- **Zero Libc Runtime Dependencies**: Uses strictly standard freestanding integer types (`uint8_t`, `uint16_t`, `uint32_t`).
- **Tri-Mode HAL Compatibility**: Functions safely when embedded in microcontrollers, emulators, or UEFI stubs.

## 6. Code Example: Invoking INT 10h to Set Video Mode

```c
#include "bios/bios.h"

void set_bios_mode_3(BiosContext *bios) {
    BiosRegisters regs;
    regs.ah = 0x00; /* Set Video Mode */
    regs.al = 0x03; /* 80x25 16-color text mode */

    bios_interrupt(bios, 0x10, &regs);
}
```
