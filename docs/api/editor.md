# Editor Integration API Reference

Header File: [`include/editor.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/editor.h)

## Overview
Binds editor modes (MS-DOS EDIT clone, Vi, WordStar) to standard virtual console widgets.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `tui_multiplexer_init` | `void` | `void` |
| `tui_multiplexer_shutdown` | `void` | `void` |
| `editor_manager_init` | `void` | `VMContext *vm` |
| `editor_manager_run` | `int` | `VMContext *vm, const char *editor_name, const char *filename` |
| `mod_ws_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_vi_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_edit_main` | `int` | `VMContext *vm, const char *filename` |
| `mod_edlin_main` | `int` | `VMContext *vm, const char *filename` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "editor.h"

void start_edit() {
    // Editor start
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
