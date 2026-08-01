# Tutorial: How to Add a Virtual Device

> **Purpose**: Guide to building and registering custom virtual hardware/devices into the BASIC++ virtual bus architecture.

---

## Step 1: Complete Custom Device Program
Below is a complete device definition and registration sequence:
```c
#include "bpp_vdev.h"
#include <stdio.h>
#include <stdint.h>

static int my_vdev_putc(VDev *dev, int c) {
    (void)dev;
    return putchar(c);
}

static int my_vdev_getc(VDev *dev) {
    (void)dev;
    return (uint8_t)getchar();
}

void register_my_device(VDevContext *ctx) {
    VDev dev = {0};
    dev.name = "CUSTOM:";
    dev.ops.putc = my_vdev_putc;
    dev.ops.getc = my_vdev_getc;
    vdev_register(ctx, dev);
}
```
