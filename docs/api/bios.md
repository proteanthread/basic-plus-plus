# `bios.h` — Authentic IBM PC/XT/AT/PCjr BIOS Micro-Library API Reference

Header File: [`engine/include/bios/bios.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/include/bios/bios.h)

## 1. Overview
The `bios.h` API provides strict C17 authentic IBM PC (5150), XT (5160), AT (5170), and PCjr (4860) firmware emulation, 1MB real-mode address mapping, BDA structure (`0x400`), IVT vector table (`0x000-0x3FF`), hardware port I/O registers, software interrupts (INT 10h-1Ah, INT 21h, INT 33h, VBE 2.0/3.0, EMS 4.0, XMS 3.0), POST code logging (`Port 0x80`), and historical BIOS revision management.

## 2. Function Index

| Function | Return Type | Parameter Signature | Description |
|----------|-------------|---------------------|-------------|
| `bios_create` | `BiosContext*` | `BiosModel model` | Allocate and initialize a new BIOS context handle. |
| `bios_destroy` | `void` | `BiosContext* ctx` | Release BIOS context resources and 1MB memory payload. |
| `bios_init` | `bool` | `BiosContext* ctx` | Initialize BDA, IVT vectors, and ROM signatures. |
| `bios_set_model` | `void` | `BiosContext* ctx, BiosModel model` | Set active hardware model (PC, XT, AT, PCjr). |
| `bios_get_model` | `BiosModel` | `const BiosContext* ctx` | Query active hardware model. |
| `bios_set_revision` | `void` | `BiosContext* ctx, BiosRevision revision` | Set historical BIOS release revision. |
| `bios_get_revision` | `BiosRevision` | `const BiosContext* ctx` | Query active historical BIOS revision. |
| `bios_get_part_number` | `const char*` | `const BiosContext* ctx` | Query IBM part number string for active revision. |
| `bios_set_clock_mode` | `void` | `BiosContext* ctx, BiosClockMode mode` | Configure hybrid speed governor mode. |
| `bios_get_clock_mode` | `BiosClockMode` | `const BiosContext* ctx` | Query speed governor mode. |
| `bios_set_clock_freq` | `void` | `BiosContext* ctx, double mhz` | Set CPU clock frequency target in MHz. |
| `bios_get_clock_freq` | `double` | `const BiosContext* ctx` | Query CPU clock frequency in MHz. |
| `bios_peek` | `uint8_t` | `BiosContext* ctx, uint32_t addr` | PEEK real-mode physical 20-bit address. |
| `bios_poke` | `void` | `BiosContext* ctx, uint32_t addr, uint8_t val` | POKE value into 20-bit address (guards ROM). |
| `bios_poke_raw` | `void` | `BiosContext* ctx, uint32_t addr, uint8_t val` | POKE raw value into memory (bypasses ROM guard). |
| `bios_inp` | `uint8_t` | `BiosContext* ctx, uint16_t port` | Read 8-bit value from hardware I/O bus port. |
| `bios_out` | `void` | `BiosContext* ctx, uint16_t port, uint8_t val` | Write 8-bit value to hardware I/O bus port. |
| `bios_interrupt` | `bool` | `BiosContext* ctx, uint8_t int_num, BiosRegs* regs` | Dispatch software interrupt service. |
| `bios_register_interrupt` | `bool` | `BiosContext* ctx, uint8_t int_num, BiosIntHandlerFn handler, void* user_data` | Register custom interrupt vector handler. |
| `bios_post_code` | `void` | `BiosContext* ctx, uint8_t code` | Log POST diagnostic code to Port 0x80. |
