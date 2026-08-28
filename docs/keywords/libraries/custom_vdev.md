# `custom_vdev` Custom Virtual Device Template

## 1. Architectural Purpose & Overview

The `custom_vdev` guide illustrates how to implement a custom virtual device backend (`VDevDriver`) attached to virtual device bus channels.

### Implementation Blueprint (C17):
```c
#include "device/vdev.h"

typedef struct CustomVDevDriver {
    int (*open)(void *ctx, const char *path, int mode);
    int (*read)(void *ctx, void *buf, size_t len);
    int (*write)(void *ctx, const void *buf, size_t len);
    int (*ioctl)(void *ctx, int cmd, void *arg);
    void (*close)(void *ctx);
} CustomVDevDriver;
```
