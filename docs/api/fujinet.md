# FujiNet Emulation Layer API Reference

Header File: [`include/fujinet.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/fujinet.h)

## Overview
Mimics retro Atari/Commodore FujiNet network interface device specifications.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `fujinet_init_system` | `void` | `VMContext *vm` |
| `fujinet_shutdown_system` | `void` | `void` |
| `fujinet_create_n_dev` | `VDev` | `VMContext *vm` |
| `fujinet_create_fuji_dev` | `VDev` | `VMContext *vm` |
| `fujinet_create_clock_dev` | `VDev` | `VMContext *vm` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "fujinet.h"

void init_fujinet() {
    fujinet_initialize();
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
