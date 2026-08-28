# `vcon` Virtual Console Subsystem (`libkernel`)

## 1. Architectural Purpose & Overview

The `vcon` subsystem (`engine/src/device/vcon.c`) manages virtual terminal state, screen buffer dimensions, text colors, cursor tracking (`CSRLIN`, `POS`), and keyboard input buffers.

### Key Architectural Invariants:
- **Persistent State Read-Back**: Handlers that depend on cursor position or screen dimensions must query `VConContext` directly at the start of execution.
- **7-Bit ASCII Output**: Pure ASCII terminal formatting ensures identical display across all target platforms.

---

## 2. Technical API Signatures (C17)

```c
VConContext *vcon_create(int cols, int rows);
void vcon_destroy(VConContext *vcon);
void vcon_get_cursor(VConContext *vcon, int *row, int *col);
void vcon_set_cursor(VConContext *vcon, int row, int col);
void vcon_cls(VConContext *vcon);
```
