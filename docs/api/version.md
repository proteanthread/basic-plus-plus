# Version Subsystem API Reference

Header File: [`include/bpp_version.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_version.h)

## Overview
Exposes compilation date, version strings, build tags, and feature flags.

## Exposed API Entities
## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_version.h"
#include <stdio.h>

void print_version() {
    printf("Version: %s\n", bpp_version_string());
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
