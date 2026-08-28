# BIOS Introspection & Hardware State Reference

The `BIOS` introspection facilities in BASIC++ v6.5.2 (`libkernel` / `libhardware`) provide runtime visibility into virtualized real-mode x86 BIOS registers, memory map layouts, interrupt vector tables, and retro hardware profiles.

## Introspection Architecture

The virtual BIOS subsystem (`engine/src/statements/system/bios.c` and `engine/include/bios/bios.h`) maintains a complete architectural state model:

```
+---------------------------------------------------------------+
|                    BASIC++ Virtual Machine                    |
|        MEMMAP$, BIOS int_num%, VARPTR(AX%), DEF SEG = &H0040  |
+-------------------------------+-------------------------------+
|                 Virtual BIOS Introspection Core               |
|            (engine/include/bios/bios.h / bios.c)              |
+---------------+---------------+---------------+---------------+
| Interrupt Map | Memory Model  |  BDA Video    | VRAM Trap Bus |
| (IVT 0x0000)  | (IBMPC..PCJR) | (0x0449/0450) | (0xB800/A000) |
+---------------+---------------+---------------+---------------+
```

---

## Introspective Functions & Features

### 1. Active Architecture Query (`MEMMAP$`)
Returns the active hardware profile string (`"IBM_PC"`, `"IBM_XT"`, `"IBM_AT"`, `"IBM_PCJR"`):
```basic
10 CurrentArch$ = MEMMAP$
20 PRINT "Active Architecture: "; CurrentArch$
```

### 2. Register State Inspection
BIOS interrupts execute against standard BASIC integer variables:
- `AX%` — Accumulator (`AH% = AX% \ 256`, `AL% = AX% AND 255`)
- `BX%` — Base register
- `CX%` — Count register
- `DX%` — Data / I/O register
- `FLAGS%` — Status flags (Carry flag bit 0 indicates interrupt error status)

```basic
100 REM Query Video Mode via INT 10h, AH=0Fh
110 AX% = &H0F00 : REM AH=0Fh, AL=00h
120 BIOS &H10
130 CurrentMode% = AX% AND 255      : REM AL = Active video mode
140 TextColumns% = (AX% \ 256) AND 255 : REM AH = Screen columns
150 ActivePage%  = (BX% \ 256) AND 255 : REM BH = Active display page
160 PRINT "Video Mode: "; CurrentMode%; " Columns: "; TextColumns%; " Page: "; ActivePage%
```

### 3. BIOS Data Area (BDA) Memory Map Introspection
The BIOS Data Area at segment `&H0040` contains standard PC hardware status words:
- `PEEK(&H0010)` — Equipment list word (installed drives, display type).
- `PEEK(&H0013)` — Base memory size in KB (up to 640 KB).
- `PEEK(&H0017)` — Keyboard shift flags (Caps Lock, Num Lock, Shift, Ctrl, Alt).

```basic
10 DEF SEG = &H0040
20 ShiftFlags% = PEEK(&H0017)
30 IF (ShiftFlags% AND 64) THEN PRINT "Caps Lock is ON"
40 IF (ShiftFlags% AND 32) THEN PRINT "Num Lock is ON"
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | Invalid interrupt number or unmapped vector |
| 57 | Device I/O Error (`ERR_DEVICE_IO_ERROR`) | Virtual BIOS subsystem not initialized (`SUPPORT_BIOS=0`) |
| 70 | Permission Denied (`ERR_PERMISSION_DENIED`) | Sandbox restrictions on low-level BIOS inspection |

---

## Cross-References

- **`MEMMAP$`** — Function returning active hardware model name.
- **`Devices_Hardware/virtual_mach/bios.md`** — BIOS interrupt execution reference.
- **`Devices_Hardware/virtual_mach/VMEM.md`** — Virtual memory and segment mapping.
- **`IBM_BIOS_Porting.md`** — Freestanding BIOS subsystem architecture.

---

## Proposed Expansion or Changes

1. **`BIOSTATUS$(int_num%)` Function**: Provide an introspection string query returning detailed vector routing and device handler state for any interrupt.
2. **BDA Register Snapshot**: Support `BIOS DUMP` to print an annotated diagnostic table of all BDA memory locations (`0x0400..0x04FF`).
