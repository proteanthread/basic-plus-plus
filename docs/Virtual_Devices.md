<!--
Title:        Virtual_Devices
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/bus.c, engine/src/device/vdev.c, engine/src/device/vcon.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Virtual Devices & Bus Architecture

The technical specification and architectural manual for the Virtual Device (VDev) subsystem, bus routing, platform console drivers, and device aliasing in BASIC++.

---

## 1. The Virtual Device Bus

All I/O operations in BASIC++ are virtualized through the Virtual Device (VDev) layer. Statement handlers never invoke raw host C library functions (`printf`, `fopen`, `socket`) directly. Instead, they call VDev API functions, which route the operation through the device bus to the appropriate device driver.

The device bus is implemented in `engine/src/device/bus.c` and the VDev manager in `engine/src/device/vdev.c`. The bus provides a uniform interface for device registration, discovery, configuration, and I/O dispatch.

---

## 2. Device Types and Drivers

BASIC++ defines standard virtual device categories:

1. **VCon (Virtual Console)**: The primary text output and input device (`engine/src/device/vcon.c`). Manages cursor coordinates, screen color attributes, dimensions, scrolling, and keystroke queuing.
2. **File Devices**: Managed through `FileContext` and routed via the platform filesystem abstraction (`engine/src/platform/plat_fs.c`).
3. **VNet (Virtual Network)**: Non-blocking TCP and UDP socket operations (`engine/src/runtime/vnet.c`).
4. **VFS (Virtual Filesystem)**: Abstraction layer providing path normalization, virtual mount points, and security sandboxing (`engine/src/runtime/vfs.c`).
5. **Graphics Devices**: SDL2-based graphics output for `SCREEN` modes (`engine/src/device/gfx.c`).
6. **BGI Devices**: The BASIC++ Graphics Interface rasterizer for pixel-level graphics (`engine/src/device/bgi/`).
7. **BIOS Devices**: PC BIOS emulation for INT 10h (video), INT 13h (disk), INT 16h (keyboard), and INT 1Ah (time) in `engine/src/bios/`.
8. **FujiNet Device**: Emulation of the FujiNet network adapter for retro-computing peripheral networking (`engine/src/device/fujinet.c`).

---

## 3. Device Registration & Driver Lifecycle

Each device type registers with the bus by providing a device descriptor containing:
- A unique device name and type identifier
- Function pointers for `init`, `shutdown`, `read`, `write`, `status`, and `control` operations
- A private context pointer for device-specific state

Devices are initialized during the boot sequence (`libboot`). The boot controller calls each device's initialization function in dependency order. Devices that depend on other subsystems (such as BGI depending on VCon for text window coordinate clipping) are initialized after their prerequisites.

Device shutdown occurs in reverse order during interpreter exit. Each device's shutdown function releases allocated memory buffers, flushes pending I/O queues, and closes active network connections.

---

## 4. Device Aliasing, Mounting & Path Redirection

Device aliases allow programs written for different platforms to run without source modifications by mapping standard shorthand identifiers to concrete targets.

### Standard Shorthand Names

| Device Name | Target Subsystem | Description |
|:---|:---|:---|
| `CON:` | Virtual Console (VCon) | Stdin / stdout; reading accepts keyboard input, writing outputs to screen. |
| `NUL:` | Null Device | Discards all output; reading returns immediate end-of-file (EOF). |
| `SCRN:` | VCon Display | Screen output only; identical to `CON:` for output, cannot be read. |
| `KYBD:` | VCon Keyboard | Keyboard input only; identical to `CON:` for input, cannot be written. |
| `LPT1:` | Line Printer 1 | Default parallel line printer channel. |
| `COM1:` | Serial Port 1 | First asynchronous serial communication port. |

### Drive Letter Aliases (`MOUNT` and `UMOUNT`)

On operating systems without physical drive letters (Linux, macOS), the VFS mount system provides drive letter emulation:

```basic
10 MOUNT "A:" TO "/home/user/basic/floppy_a"
20 MOUNT "C:" TO "/home/user/basic"
30 OPEN "A:MYFILE.BAS" FOR INPUT AS #1
40 UMOUNT "A:"
```

### Output Stream Redirection (`REDIRECT`)

The `REDIRECT` statement temporarily redirects standard device streams to a file or alternate device:

```basic
10 REDIRECT SCRN: TO "output.txt"
20 PRINT "This text is redirected to the file"
30 REDIRECT SCRN: TO CON:
40 PRINT "This text returns to the console screen"
```

---

## 5. Virtual Printer & PDF Devices

BASIC++ includes a freestanding, zero-dependency text-to-PDF pseudo-printer driver registered across standard printer device prefixes:
- **Line Printers**: `LPT:`, `LPT1:` through `LPT8:`
- **Standard Printers**: `PRN:`, `PRN1:` through `PRN8:`
- **CP/M Listing Devices**: `LST:`, `LST1:` through `LST8:`
- **Direct PDF Devices**: `PDF:`, `PDF1:` through `PDF8:`

### Features and Usage:

```basic
10 OPEN "LPT1:invoice.pdf" FOR OUTPUT AS #1
20 PRINT #1, "INVOICE #1042"
30 PRINT #1, "Date: 2026-09-08"
40 PRINT #1, CHR$(12)   : REM Form Feed creates a new page
50 PRINT #1, "Page 2 - Line Items"
60 CLOSE #1
```

- **Zero External Dependencies**: Direct PostScript Type 1 Courier font metric calculation and PDF 1.4 stream serialization without external libraries.
- **Automatic Extension**: Appends `.pdf` automatically if omitted in the filename.
- **Form Feed Pagination**: Form Feed (`CHR$(12)` / ``) automatically splits document content across multiple PDF pages.
