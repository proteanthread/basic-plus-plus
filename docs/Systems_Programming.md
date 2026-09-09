<!--
Title:        Systems_Programming
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/src/bios/, engine/src/statements/system/
Generated:    no, manual system specification
Status:       current
-->

# BASIC++ v6.5.2 Systems Programming Reference

The comprehensive guide to systems-level programming in BASIC++, covering virtual BIOS emulation, memory manipulation statements (`PEEK`, `POKE`), I/O port virtualization (`INP`, `OUT`), interrupt hooks, and machine language interfaces.

---

## 1. Overview of Systems Architecture

BASIC++ provides systems-level programming capabilities through virtual device interfaces, memory manipulation statements, BIOS emulation, and direct hardware virtualization. These facilities allow BASIC++ programs to perform low-level tasks traditionally reserved for C or assembly language, while maintaining the engine's security guarantees through the virtual device and security layers.

---

## 2. Memory Manipulation: `PEEK` and `POKE`

`POKE address, value` writes a single byte (0-255) to the specified address in the virtual BIOS emulation memory space. `PEEK(address)` reads a byte from that address:

```basic
10 POKE &H0449, 3       ' Set BIOS video mode to 3 (80x25 text mode)
20 Mode = PEEK(&H0449)  ' Read current video mode from BIOS Data Area
30 PRINT "Video mode:"; Mode
```

The BIOS memory space mirrors the standard IBM PC memory layout:
- **0x0000 - 0x03FF**: Interrupt Vector Table (IVT).
- **0x0400 - 0x04FF**: BIOS Data Area (BDA).
- **0xA0000 - 0xAFFFF**: Enhanced Graphics Video RAM (EGA/VGA).
- **0xB8000 - 0xBFFFF**: Color Text Mode Video RAM (CGA/VGA).

In hosted executables (`baspp`, `bpp`, `bs`), `POKE` and `PEEK` operate inside a safe virtualized memory sandbox (`libhardware`/`vmem`) and cannot corrupt the host operating system. In standalone systems compilation (`bppc --freestanding`), physical memory operations emit volatile pointer dereferences with compiler memory barriers.

---

## 3. Virtual Hardware I/O Ports: `INP` and `OUT`

`INP(port)` reads a byte from a virtualized hardware I/O port. `OUT port, value` writes a byte to the specified port:

```basic
10 OUT &H3D4, 14         ' CRT controller: select cursor high byte register
20 OUT &H3D5, 0          ' Set cursor high byte to 0
30 OUT &H3D4, 15         ' CRT controller: select cursor low byte register
40 OUT &H3D5, 80         ' Set cursor low byte to 80 (screen offset)
```

Virtual I/O ports are routed through the VHAL bus to simulated peripherals (CRT controller, PIT timer 8253, PIC interrupt controller 8259A).

---

## 4. BIOS Interrupt Emulation Services

The BIOS emulation subsystem (`engine/src/bios/`) provides virtual implementations of classic IBM PC interrupts:
- **INT 10h**: Video services (mode selection, cursor positioning, scroll regions).
- **INT 13h**: Disk sector read/write services.
- **INT 16h**: Keyboard input and buffer polling.
- **INT 1Ah**: Real-time clock and timer services.

---

## 5. Machine Language Invocation: `SYS` and `USR`

`SYS address` transfers execution to a virtual machine subroutine at the specified memory address.

`USR(n)` invokes a user-defined machine language callback registered at slot `n`. USR callbacks receive a single numeric parameter and return a numeric result. In modern builds, USR functions are registered as C callbacks via the host interop layer.

---

## 6. Variable Pointers: `VARPTR` and `VARPTR$`

`VARPTR(variable)` returns the internal memory address of a variable's storage descriptor.

`VARPTR$(variable)` returns a three-byte string encoding the type descriptor and internal offset, preserving vintage QuickBASIC binary format compatibility.
