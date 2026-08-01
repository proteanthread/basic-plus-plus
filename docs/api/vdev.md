# Virtual Device framework API Reference

Header File: [`include/bpp_vdev.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_vdev.h)

## Overview
Exposes virtual hardware/device operations hooks (VDevOps) for custom peripherals.

## Exposed API Entities
### Structs & Types
- `VDev VDev`
- `VDevContext VDevContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vdev_shutdown` | `void` | `VDevContext *ctx` |
| `vdev_register` | `bool` | `VDevContext *ctx, VDev dev` |
| `vdev_printf` | `int` | `VDevContext *ctx, const char *fmt, ...` |
| `vdev_puts` | `int` | `VDevContext *ctx, const char *s` |
| `vdev_putc` | `int` | `VDevContext *ctx, int c` |
| `vdev_play_beep` | `void` | `VDevContext *ctx` |
| `vdev_music_queue_length` | `int` | `void` |
| `vdev_music_clear` | `void` | `void` |
| `vdev_play_sound_freq` | `void` | `double freq, double duration_seconds` |
| `vdev_gfx_poll_events` | `void` | `void` |
| `vdev_gfx_enable` | `void` | `bool allowed, bool gui_boot` |
| `vdev_gfx_boot_check` | `void` | `struct VMContext *vm` |
| `gfx_get_char_at` | `int` | `int row, int col` |
| `gfx_get_attr_at` | `int` | `int row, int col` |
| `vdev_read` | `int` | `VDev *d, void *buf, int len` |
| `vdev_write` | `int` | `VDev *d, const void *buf, int len` |
| `vdev_seek` | `long` | `VDev *d, long offset, int whence` |
| `vdev_ioctl` | `int` | `VDev *d, int cmd, void *arg` |
| `vdev_status` | `int` | `VDev *d` |
| `vdev_poll` | `int` | `VDev *d` |
| `vdev_count` | `int` | `VDevContext *ctx` |
| `vdev_list_all` | `void` | `VDevContext *ctx` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_vdev.h"

extern VDev vdev_console_create(void);

void init_vdev(VMContext *vm) {
    vdev_register(vm_get_vdev(vm), vdev_console_create());
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
