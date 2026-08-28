# `bus` Virtual Hardware Device Bus (`libkernel`)

## 1. Architectural Purpose & Overview

The `bus` subsystem (`engine/src/device/bus.c`) implements an abstracted expansion bus connecting the VM CPU loop to virtual hardware devices, peripheral controllers, and I/O ports (`INP` / `OUT`).

### Key Architectural Invariants:
- **Port Mapping**: I/O addresses ($0\text{ to }65535$) route through port read/write handler tables.
- **Device Registration**: Peripherals register with unique device IDs and interrupt lines.

---

## 2. Technical API Signatures (C17)

```c
void bus_init(void);
uint8_t bus_read_port(uint16_t port);
void bus_write_port(uint16_t port, uint8_t val);
```
