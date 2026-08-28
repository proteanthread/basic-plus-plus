# `bios` Virtual Machine BIOS Subsystem (`libbios` / `libkernel`)

## 1. Architectural Purpose & Overview

The `bios` subsystem (`engine/src/bios/bios.c`) emulates standard IBM PC/XT/AT and retro system BIOS interrupt services (`INT 10h` Video, `INT 16h` Keyboard, `INT 1Ah` RTC, `INT 13h` Disk).

### Key Architectural Invariants:
- **Freestanding C17**: Zero libc dependencies.
- **BDA Video Registers**: Authoritative BDA states at `0x0449` (video mode), `0x0450` (cursor pos).
- **Tri-Mode CPU Engine**: C17 HLE, micro-8086 interpreter (`libcpu8086`), or host CPU callbacks.

---

## 2. Technical API Signatures (C17)

```c
void bios_init(void);
void bios_interrupt(uint8_t int_no, BiosRegisters *regs);
```
