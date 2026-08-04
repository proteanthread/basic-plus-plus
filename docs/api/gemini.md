# Gemini Protocol client API Reference

Header File: [`include/gemini.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/gemini.h)

## Overview
Implements sandboxed lightweight HTTP/Gemini protocol handlers.

## Exposed API Entities
## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "gemini.h"

void get_gemini() {
    gemini_request("gemini://gemini.circumlunar.space");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
