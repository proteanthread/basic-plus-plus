# Boot Subsystem API Reference

Header File: [`include/bpp_boot.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_boot.h)

## Overview
Handles the bootstrapping of the BASIC++ environment, initializing the VM and virtual devices.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `boot_shutdown` | `void` | `BootContext *ctx` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_boot.h"

int main() {
    BootConfig config;
    memset(&config, 0, sizeof(config));
    config.is_repl = false;
    config.prog_mem = 1024 * 1024;
    config.var_mem = 1024 * 1024;
    config.str_mem = 1024 * 1024;
    config.scratch_mem = 1024 * 1024;
    
    BootContext *boot = boot_execute(&config);
    if (!boot) {
        return 1;
    }
    boot_shutdown(boot);
    return 0;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
