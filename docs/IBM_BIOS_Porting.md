<!--
Title:        IBM BIOS Porting
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/bios/, engine/include/bios/
Generated:    no
Status:       Active
-->

# BASIC++ IBM BIOS Emulation Architecture

The architectural reference detailing the IBM PC/XT/AT BIOS emulation subsystem (`engine/src/bios/`), BIOS Data Area mapping, interrupt vectors, and virtual Video RAM.

## 1. Architectural Role & Overview

The BIOS emulation subsystem reproduces the memory maps, I/O ports, and interrupt services of vintage IBM PC, XT, and AT computers. This enables vintage programs utilizing direct memory reads (`PEEK`), direct writes (`POKE`), hardware port communication (`INP`, `OUT`), and software interrupts (`CALL INTERRUPT`) to run unmodified across modern operating systems.

### 1.1 Machine Personalities
Emulation personalities are implemented in dedicated source modules:
- `bios_pc.c`: IBM PC Model 5150 emulation (Intel 8088, cassette port, 64-256KB base RAM).
- `bios_xt.c`: IBM PC/XT Model 5160 (fixed disk drive controller, 8 expansion slots).
- `bios_at.c`: IBM PC/AT Model 5170 (Intel 80286, 16-bit AT bus, CMOS RTC).
- `bios_jr.c`: IBM PCjr Model 4860 (extended sound generator, cartridge slots).

---

## 2. Emulated BIOS Interrupt Services

The subsystem virtualizes foundational PC interrupt vectors:

1. **INT 10h — Video Services (`bios_int10.c`)**:
   - `AH=00h`: Set video mode (Modes 0–13h).
   - `AH=01h`: Set cursor shape and scanline range.
   - `AH=02h`: Set cursor position (`row`, `col`).
   - `AH=03h`: Read cursor position and configuration.
   - `AH=06h` / `AH=07h`: Scroll window up / down.
   - `AH=08h` / `AH=09h`: Read / write character and attribute at cursor.
   - `AH=0Eh`: Teletype output (TTY character write with auto-wrap).
   - `AH=10h`–`13h`: EGA/VGA palette registers and string writes.

2. **INT 13h — Virtual Disk Services (`bios_int13.c`)**:
   - Emulates diskette drive read/write sectors against virtual disk images.

3. **INT 16h — Keyboard Services (`bios_int16.c`)**:
   - `AH=00h`: Blocking read character from keyboard buffer.
   - `AH=01h`: Check buffer status (non-blocking query for `INKEY$`).
   - `AH=02h`: Read shift key flags byte (`0x0417`).

4. **INT 1Ah — Time-of-Day Services (`bios_int1a.c`)**:
   - `AH=00h`: Read system timer tick count (`0x046C`).
   - `AH=01h`: Set system timer tick count.
   - `AH=02h` / `AH=04h`: Real-Time Clock read time and calendar date.

---

## 3. BIOS Data Area (BDA) Memory Mapping

The BIOS Data Area at physical memory `0x0400`–`0x04FF` is mapped into the VM memory space:

| Address | Size | Description | Vintage Usage |
| :--- | :--- | :--- | :--- |
| `0x0449` | 1 byte | Current video display mode | `mode = PEEK(&H0449)` |
| `0x044A` | 2 bytes | Screen column width (40 or 80) | `cols = PEEK(&H044A)` |
| `0x044E` | 2 bytes | Active video page offset | Memory offset to current page |
| `0x0450` | 16 bytes | Cursor positions (8 video pages × 2 bytes) | Page row and column pairs |
| `0x0460` | 2 bytes | Cursor scanline start and end lines | Cursor shape configuration |
| `0x0462` | 1 byte | Active display page number | `page = PEEK(&H0462)` |
| `0x0463` | 2 bytes | CRT controller base I/O port (`0x3B4` / `0x3D4`) | Monochrome vs Color adapter |
| `0x046C` | 4 bytes | Daily timer tick counter (18.2065 Hz) | `ticks = PEEK(&H046C)` |
| `0x0470` | 1 byte | 24-hour timer overflow rollover flag | Midnight transition sentinel |
| `0x0484` | 1 byte | Character rows minus 1 (EGA/VGA) | 24 for 25-line, 42 for 43-line |

---

## 4. Virtual Video RAM Buffers

The subsystem allocates virtual Video RAM windows monitored by observer callbacks:
- **`0xB8000`–`0xBFFFF` (CGA/EGA/VGA Text Mode)**: 32 KB character cell buffer (alternating character byte and attribute byte). Writes to `0xB8000` automatically trigger visual refresh on the active console or terminal.
- **`0xA0000`–`0xAFFFF` (EGA/VGA Graphics Mode)**: 64 KB planar graphics framebuffer trapped and rendered via the BGI software rasterizer.
