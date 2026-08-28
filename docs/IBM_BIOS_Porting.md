# BASIC++ v6.5.2 IBM BIOS Porting Guide

## 1. OVERVIEW

The BIOS emulation subsystem (engine/src/bios/) provides a virtual implementation of the IBM PC/XT/AT BIOS. This allows BASIC++ programs that use POKE, PEEK, INP, OUT, and BIOS interrupt-level operations to run on any platform. The BIOS emulation faithfully reproduces the IBM PC memory map, I/O port behavior, and interrupt services.

## 2. EMULATED BIOS SERVICES

**INT 10h — Video Services**: Mode setting (AH=00h), cursor shape (AH=01h), cursor position (AH=02h), read cursor (AH=03h), scroll up (AH=06h), scroll down (AH=07h), read character (AH=08h), write character (AH=09h, 0Ah), set palette (AH=0Bh), write pixel (AH=0Ch), read pixel (AH=0Dh), TTY output (AH=0Eh), get video mode (AH=0Fh), EGA/VGA functions (AH=10h-13h).

**INT 13h — Disk Services**: Virtual disk read/write for programs that access disk through BIOS calls.

**INT 16h — Keyboard Services**: Read key (AH=00h), check buffer (AH=01h), shift state (AH=02h), extended functions (AH=10h-12h).

**INT 1Ah — Time Services**: Read clock (AH=00h), set clock (AH=01h), read date (AH=04h), set date (AH=05h).

## 3. BIOS DATA AREA

The BIOS Data Area (BDA) at 0x0400-0x04FF is fully emulated. Key fields:

| Address | Size | Description |
|---------|------|-------------|
| 0x0449 | 1 | Current video mode |
| 0x044A | 2 | Screen width in columns |
| 0x044E | 2 | Current video page offset |
| 0x0450 | 16 | Cursor positions (8 pages × 2 bytes) |
| 0x0460 | 2 | Cursor shape (start/end scan lines) |
| 0x0462 | 1 | Active display page |
| 0x0463 | 2 | CRT controller base port (3B4h or 3D4h) |
| 0x0465 | 1 | CGA mode register value |
| 0x0466 | 1 | CGA color register value |
| 0x046C | 4 | Timer tick count |
| 0x0470 | 1 | Timer overflow flag |
| 0x0471 | 1 | Ctrl+Break flag |
| 0x0484 | 1 | EGA/VGA rows minus 1 |

Programs can read these fields with PEEK and write them with POKE:

```basic
10 Mode = PEEK(&H0449)          ' Read current video mode
20 Cols = PEEK(&H044A)          ' Read screen width
30 POKE &H0462, 1               ' Switch to video page 1
```

## 4. VIDEO RAM

The BIOS emulation provides virtual video RAM:

**0xB8000-0xBFFFF**: CGA/EGA/VGA text-mode framebuffer. Each character cell uses 2 bytes (character code + attribute). Writing to this region updates the virtual display through BiosVRAMObserver callbacks.

**0xA0000-0xAFFFF**: EGA/VGA graphics framebuffer. Writing pixel data to this region is trapped and rendered through the BGI rasterizer.

```basic
10 ' Write "A" in white on blue at position (0,0) in text mode
20 POKE &HB8000, 65              ' Character "A"
30 POKE &HB8001, &H1F            ' Attribute: white on blue
```

## 5. I/O PORTS

The BIOS emulation virtualizes key I/O ports:

| Port | Description |
|------|-------------|
| 3B4h-3B5h | MDA CRT controller |
| 3D4h-3D5h | CGA/EGA/VGA CRT controller (6845 CRTC) |
| 3C0h | VGA attribute controller |
| 3C4h-3C5h | VGA sequencer |
| 3C7h-3C9h | VGA DAC (palette) |
| 3CEh-3CFh | VGA graphics controller |
| 3DAh | CGA/VGA status register |
| 60h | Keyboard data port |
| 61h | System control port |
| 40h-43h | PIT (timer) |

## 6. TRI-MODE HAL DISPATCH

The BIOS subsystem implements the Tri-Mode HAL Dispatch system:

**STATIC_INLINE** — For IoT/embedded microcontrollers where BIOS functions are compiled as inline code for minimum overhead.

**PLUGGABLE_STRUCT** — For host emulators and BASIC++ desktop builds where BIOS functions are called through function pointers, allowing runtime replacement.

**MACRO_OVERRIDE** — For FreeDOS/UEFI builds where BIOS calls are redirected to real hardware through preprocessor macros.

## 7. CPU EMULATION

The BIOS subsystem includes a micro-8086 interpreter (libcpu8086) for executing x86 machine code in BIOS ROM routines. This enables programs that call real BIOS routines (through SYS or USR) to execute the actual x86 instructions in a sandboxed environment.

## 8. PORTING TO NEW PLATFORMS

When porting BASIC++ to a new platform, the BIOS emulation layer requires no changes — it is a pure software emulation. The platform layer (plat_console.c, plat_fs.c, etc.) provides the actual hardware interface. The BIOS emulation sits between BASIC++ statements and the platform layer, translating POKE/PEEK/INP/OUT operations into platform-appropriate calls.
