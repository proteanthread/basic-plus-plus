<!--
Title:        Virtual_Memory_RAMBANKs
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/memory/vmem_mmu.c, engine/src/memory/segmented_mem.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Virtual Memory & RAM Banks Architecture

The authoritative specification for segmented virtual memory (`vmem`), bank-switched RAM banking, and memory management unit (MMU) virtualization in BASIC++ v6.5.2.

---

## 1. Segmented Memory Architecture (`vmem`)

The segmented memory subsystem (`engine/src/memory/vmem_mmu.c`) provides access to an address space partitioned into switchable 64 KB memory banks. This emulates classic bank-switching memory architectures (EMS 4.0, XMS, Commodore REU, Apple II language cards) while running on 32-bit and 64-bit host architectures:

- **Memory Pools by Binary Edition**:
  - `baspp.exe` (Desktop Flagship): 640 MB memory pool with up to 1024 switchable virtual RAM banks.
  - `bpp.exe` (Lite Edition): 384 MB flat pool; segmented memory excluded to optimize terminal execution.
  - `bs.exe` (Batch Runner): 64 MB flat pool.
  - `iot.exe` (Microcontroller Edition): 2 MB pool.

---

## 2. Bank Switching Operations

- **`BANK bank_num`**: Switches the active 64 KB memory bank mapped into the addressable window. Subsequent `PEEK`, `POKE`, `BLOAD`, and `BSAVE` operations target the selected bank.
- **`RAMBANK bank_num`**: Selects a high-memory extended RAM bank.
- **`MEM[...]`**: Array-style physical memory access mapped into the active bank window.

---

## 3. Safety and Sandbox Bounds Checking

In accordance with project safety standards and the EU Cyber Resilience Act (CRA 2024):
- All memory addresses accessed via `PEEK`, `POKE`, or `BANK` are validated against allocated arena boundaries in `vmem_mmu.c`.
- Accesses outside allocated bank memory raise **Error 5: Illegal Function Call** rather than triggering host OS segmentation faults.

---

## 4. Example: Bank Switching & Memory Access

```basic
10 REM Bank Switching Memory Demo
20 BANK 1
30 POKE 100, 42
40 BANK 2
50 POKE 100, 99
60 BANK 1 : PRINT "Bank 1, Byte 100: "; PEEK(100)
70 BANK 2 : PRINT "Bank 2, Byte 100: "; PEEK(100)
```
