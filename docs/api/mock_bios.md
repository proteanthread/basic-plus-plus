# Mock BIOS emulation API Reference

Header File: [`include/mock_bios.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/mock_bios.h)

## Overview
Simulates IBM PC BIOS interrupts (INT 10h, INT 16h, INT 21h) for retro compatibility.

## Exposed API Entities
### Structs & Types
- `MockBiosRegs`
- `MockBiosContext`

### Enums
- `MockBiosModel`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `mock_bios_init_mem` | `BIOS_API void` | `MockBiosContext *ctx, uint8_t *mem_segment, size_t mem_size, MockBiosModel model` |
| `mock_bios_in` | `BIOS_API uint8_t` | `MockBiosContext *ctx, uint16_t port` |
| `mock_bios_out` | `BIOS_API void` | `MockBiosContext *ctx, uint16_t port, uint8_t val` |
| `mock_bios_interrupt` | `BIOS_API void` | `MockBiosContext *ctx, uint8_t int_num` |
| `mock_bios_model_from_string` | `BIOS_API MockBiosModel` | `const char *name` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "mock_bios.h"

void run_interrupt(MockBiosContext *ctx) {
    mock_bios_interrupt(ctx, 0x10);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
