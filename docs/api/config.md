# Configuration Subsystem API Reference

Header File: [`include/config.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/config.h)

## Overview
Handles configuration settings, limits, dial-in setups, and memory allocations for the VM instance.

## Exposed API Entities
## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "config.h"

void setup_config() {
    BootConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.prog_mem = 1024 * 1024 * 64; // 64 MB
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
