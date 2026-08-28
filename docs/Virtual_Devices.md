# BASIC++ v6.5.2 Virtual Devices

## 1. THE VIRTUAL DEVICE BUS

All I/O operations in BASIC++ are virtualized through the VDev (Virtual Device) layer. Statement handlers never call raw C library functions (printf, fopen, socket) directly. Instead, they call VDev API functions, which route the operation through the device bus to the appropriate device driver.

The device bus is implemented in engine/src/device/bus.c and the VDev manager in engine/src/device/vdev.c. The bus provides a uniform interface for device registration, discovery, and dispatch.

## 2. DEVICE TYPES

BASIC++ defines several device categories:

**VCon (Virtual Console)** — The primary text output and input device. Manages cursor position, color attributes, screen dimensions, scrolling, and key input. Implemented in engine/src/device/vcon.c.

**File Devices** — Managed through the FileContext and routed via the platform filesystem abstraction (plat_fs.c).

**VNet (Virtual Network)** — TCP and UDP socket operations. Implemented in engine/src/runtime/vnet.c.

**VFS (Virtual Filesystem)** — Abstraction layer over the host filesystem that provides path normalization, virtual mount points, and security filtering. Implemented in engine/src/runtime/vfs.c.

**Graphics Devices** — SDL2-based graphics output for SCREEN modes. Implemented in engine/src/device/gfx.c.

**BGI Devices** — The BASIC++ Graphics Interface rasterizer for pixel-level graphics. Implemented in engine/src/device/bgi/.

**BIOS Devices** — PC BIOS emulation for INT 10h (video), INT 13h (disk), INT 16h (keyboard), and INT 1Ah (time). Implemented in engine/src/bios/.

**FujiNet Device** — Emulation of the FujiNet network adapter for Atari-compatible networking. Implemented in engine/src/device/fujinet.c.

## 3. DEVICE REGISTRATION

Each device type registers with the bus by providing a device descriptor containing:
- A unique device name
- A device type identifier
- Function pointers for init, shutdown, read, write, status, and control operations
- A private context pointer for device-specific state

Custom devices can be registered at runtime through the module system.

## 4. DEVICE COMMANDS

DEVICES lists all registered virtual devices:

```basic
> DEVICES
Slot  Type      Name          Status
----  --------  -----------   ------
  0   VCon      Console       Active
  1   File      FileSystem    Active
  2   VNet      Network       Active
  3   VFS       VirtFS        Active
  4   BGI       Graphics      Idle
  5   BIOS      PCBios        Active
```

DEVMAP displays the current device slot mapping, showing which device type is assigned to each slot.

## 5. CONSOLE DEVICE (VCon)

The VCon device manages all text-mode console operations. It maintains:

- **Cursor position** — Row and column, updated by LOCATE and character output.
- **Color attributes** — Foreground and background colors, updated by COLOR.
- **Screen dimensions** — Rows and columns, updated by WIDTH and window resizing.
- **Scroll region** — Defines the scrollable area of the screen.
- **Key buffer** — Queues keystrokes for INKEY$ and INPUT.

All PRINT output goes through VCon. The VCon implementation calls platform-specific console functions (plat_console.c) for the actual terminal interaction, ensuring the same BASIC++ program produces identical output on Windows (Console API), Linux (termios/ncurses), and FreeDOS (INT 10h).

## 6. WHY VIRTUAL DEVICES

The device virtualization layer provides three critical benefits:

1. **Portability** — The same BASIC++ program works on Windows, Linux, FreeDOS, and embedded platforms without source changes. The platform layer adapts device operations to the host OS.

2. **Testability** — Virtual devices can be replaced with mock implementations for automated testing. A mock VCon captures output without requiring a real terminal.

3. **Security** — Device access can be denied based on the security level. At PARANOID level, all devices except the basic console are disabled. The security check happens at the device bus level, so no statement handler needs individual security logic.

## 7. DEVICE INITIALIZATION

Devices are initialized during the boot sequence (libboot). The boot controller calls each device's init function in dependency order. Devices that depend on other devices (e.g., BGI depends on VCon for text window rendering) are initialized after their dependencies.

Device shutdown occurs in reverse order during interpreter exit. Each device's shutdown function releases resources, flushes buffers, and closes connections.

## 8. VIRTUAL PRINTER & PDF DEVICES

BASIC++ provides a freestanding, zero-dependency text-to-PDF pseudo-printer driver registered across multiple standard device prefixes:

- **Line Printers**: `LPT:`, `LPT1:` through `LPT8:`
- **Standard Printers**: `PRN:`, `PRN1:` through `PRN8:`
- **CP/M Listing Devices**: `LST:`, `LST1:` through `LST8:`
- **Direct PDF Devices**: `PDF:`, `PDF1:` through `PDF8:`

### Syntax & Filename Specifications:
You can specify custom output filenames directly after the device prefix:
```basic
OPEN "LPT1:invoice.pdf" FOR OUTPUT AS #1
OPEN "PRN:quarterly_report" FOR OUTPUT AS #2   : REM Automatically appends .pdf
OPEN "PDF:reports/summary.pdf" FOR OUTPUT AS #3 : REM Supports relative paths/subdirectories
OPEN "LPT1:" FOR OUTPUT AS #4                   : REM Defaults to <program_name>.pdf or OUTPUT.PDF
```

### Features:
1. **Zero External Dependencies**: PostScript/PDF Type 1 Courier font rendering without host print spoolers or external font files.
2. **Automatic PDF Extension**: `.pdf` is appended automatically if omitted in the filename.
3. **Multi-Channel Scoping**: Each open channel operates an isolated PDF document, serializing upon `CLOSE #n`.
4. **Form Feed Pagination**: Form Feed (`CHR$(12)` / `\f`) automatically splits document content across multiple PDF pages.

