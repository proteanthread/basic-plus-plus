# Tutorial: Implementing Custom Virtual Devices in BASIC++

## 1. Overview

This tutorial demonstrates how to construct a custom virtual device and attach it to the master Virtual Device Bus (`device/vdev.h`).

We will implement a virtual memory-backed loopback pipe device named `PIPE:` that buffers data written via `PRINT#` or `PUT#` and allows it to be read back sequentially via `LINE INPUT#` or `GET#`.

---

## 2. The `VDev` Interface & Operations Table

A virtual device implements the `VDevOps` function pointer table:

```c
#include "device/vdev.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PIPE_BUFFER_SIZE 4096

typedef struct {
    char   buffer[PIPE_BUFFER_SIZE];
    size_t write_pos;
    size_t read_pos;
} PipeDeviceState;

/* Open Callback */
static bool pipe_open(VDev *dev, const char *path, int mode) {
    (void)path; (void)mode;
    PipeDeviceState *state = (PipeDeviceState *)dev->priv_data;
    state->write_pos = 0;
    state->read_pos = 0;
    dev->is_open = true;
    return true;
}

/* Close Callback */
static void pipe_close(VDev *dev) {
    dev->is_open = false;
}

/* Write Callback */
static size_t pipe_write(VDev *dev, const void *buf, size_t count) {
    PipeDeviceState *state = (PipeDeviceState *)dev->priv_data;
    size_t available = PIPE_BUFFER_SIZE - state->write_pos;
    size_t to_write = count < available ? count : available;
    
    memcpy(state->buffer + state->write_pos, buf, to_write);
    state->write_pos += to_write;
    return to_write;
}

/* Read Callback */
static size_t pipe_read(VDev *dev, void *buf, size_t count) {
    PipeDeviceState *state = (PipeDeviceState *)dev->priv_data;
    size_t available = state->write_pos - state->read_pos;
    size_t to_read = count < available ? count : available;

    memcpy(buf, state->buffer + state->read_pos, to_read);
    state->read_pos += to_read;
    return to_read;
}

/* Put Character Callback */
static int pipe_putc(VDev *dev, int c) {
    PipeDeviceState *state = (PipeDeviceState *)dev->priv_data;
    if (state->write_pos < PIPE_BUFFER_SIZE) {
        state->buffer[state->write_pos++] = (char)c;
        return c;
    }
    return -1;
}

/* Get Character Callback */
static int pipe_getc(VDev *dev) {
    PipeDeviceState *state = (PipeDeviceState *)dev->priv_data;
    if (state->read_pos < state->write_pos) {
        return (unsigned char)state->buffer[state->read_pos++];
    }
    return -1; /* EOF */
}
```

---

## 3. Creating and Registering the Virtual Device

```c
VDev *create_pipe_device(VDevContext *bus) {
    VDev *dev = (VDev *)calloc(1, sizeof(VDev));
    PipeDeviceState *state = (PipeDeviceState *)calloc(1, sizeof(PipeDeviceState));

    dev->name = "PIPE:";
    dev->device_class = VDCLASS_PIPE;
    dev->capabilities = VDCAP_RW | VDCAP_STREAM;
    dev->priv_data = state;

    dev->ops.open  = pipe_open;
    dev->ops.close = pipe_close;
    dev->ops.read  = pipe_read;
    dev->ops.write = pipe_write;
    dev->ops.getc  = pipe_getc;
    dev->ops.putc  = pipe_putc;

    vdev_bus_attach(bus, dev);
    return dev;
}
```

---

## 4. Testing the Virtual Device in BASIC++

```basic
10 PRINT "Opening virtual PIPE: device for writing..."
20 OPEN "PIPE:" FOR OUTPUT AS #1
30 PRINT #1, "Message 1: Engine Telemetry"
40 PRINT #1, "Message 2: Sensor Data 42.5"
50 CLOSE #1
60 
70 PRINT "Reading back from virtual PIPE: device..."
80 OPEN "PIPE:" FOR INPUT AS #2
90 WHILE NOT EOF(2)
100   LINE INPUT #2, LINE_DATA$
111   PRINT "Read from pipe: "; LINE_DATA$
120 WEND
130 CLOSE #2
```

Output:
```text
Opening virtual PIPE: device for writing...
Reading back from virtual PIPE: device...
Read from pipe: Message 1: Engine Telemetry
Read from pipe: Message 2: Sensor Data 42.5
```
