<!--
Title:        Device_Reference
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/vdev.c, engine/src/statements/io/device/stmt_device.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Virtual Device Subsystem & VHAL Master Reference

The authoritative reference catalog for the Virtual Hardware Abstraction Layer (VHAL), universal sub-device indexing laws, registered virtual devices across vintage ecosystems, programmatic device discovery, and device control statements in BASIC++.

---

## 1. Architectural Overview & Universal Sub-Device Law

The BASIC++ Virtual Device Subsystem (`libkernel` / `engine/src/device/vdev.c`) provides a unified, cross-ecosystem hardware abstraction connecting BASIC programs, C micro-libraries, and host hardware peripherals.

### Sub-Device Normalization and Indexing Rules

1. **Unindexed Normalization**: Any device referenced without an explicit sub-device number (e.g. `LOOP:`, `CAS:`, `MDM:`, `CON:`, `CONS:`, `COM:`) automatically normalizes to sub-device 1 (e.g. `LOOP1:`, `CAS1:`, `MDM1:`, `CON1:`, `CONS1:`, `COM1:`).
2. **User Sub-Channels (`DEV1:` through `DEV8:`)**: Channel slots 1 through 8 are allocated for standard user applications, peripheral ports, and communication channels.
3. **Supervisor Reservation Guard (`DEV0:` and `DEV9:`)**:
   - `DEV0:` is strictly reserved for system loopback diagnostics, supervisor channels, and the `LOGGER` syslog virtual sink.
   - `DEV9:` is strictly reserved for the master bus broadcast sink and `WALL:` messaging bus.
   - Accessing `DEV0:` or `DEV9:` via standard user `OPEN` operations triggers Error 5 (Illegal Function Call).
4. **Zero Built-In Aliases Invariant**: The engine contains zero pre-mapped built-in aliases. All device families are registered as authentic first-class drivers with distinct hardware personalities. Aliasing (`DEVICE ALIAS ... AS ...` or `DEVICE BIND ... TO ...`) is reserved strictly for user-defined mappings at runtime.

---

## 2. Master Catalog of Registered Virtual Devices

| Device Prefix | Ecosystem / Standard | Class | Description & Capabilities |
|:---|:---|:---|:---|
| `CON1:` - `CON8:` | Master Dialect | `VDCLASS_CONSOLE` | Primary Virtual Console instances (Terminal screen + keyboard). |
| `CONS1:` - `CONS8:` | Tymshare Super BASIC | `VDCLASS_CONSOLE` | Multi-user Tymshare Super BASIC console streams. |
| `TTY1:` - `TTY8:` | Unix / POSIX | `VDCLASS_CONSOLE` | Virtual Terminal communication ports multiplexing to `CONn:`. |
| `KEY1:` - `KEY8:` | Hardware | `VDCLASS_HID` | Keyboard-only input half of Virtual Console `CONn:`. |
| `SCN1:` - `SCN8:` | Hardware | `VDCLASS_DISPLAY` | Screen-only output half of Virtual Console `CONn:`. |
| `WORKSTN1:` - `WORKSTN8:` | DEC / IBM / VAX | `VDCLASS_CONSOLE` | Interactive workstation terminal streams (`WS1..8:`). |
| `TEK1:` - `TEK8:` | Tektronix 4010/4014 | `VDCLASS_CONSOLE` | Vector storage tube graphics and alpha terminal streams. |
| `52501:` - `52508:` | IBM System/3X / AS400 | `VDCLASS_CONSOLE` | Twinax 5250 / 3270 block-mode presentation space streams. |
| `LOOP1:` - `LOOP8:` | Diagnostic / VHAL | `VDCLASS_LOOPBACK` | Circular FIFO loopback testing buffers (`LOOP0:` supervisor). |
| `USER1:` - `USER8:` | Multi-User Sessions | `VDCLASS_USER` | Multi-user session messaging queues (`USER0:` syslog sink). |
| `CAS1:` - `CAS8:` | Vintage Audio / Tape | `VDCLASS_TAPE` | Cassette audio bridge for Kansas City, TRS-80, Atari, CBM FSK. |
| `TAP1:` - `TAP8:` | Tape Containers | `VDCLASS_TAPE` | Container-based magnetic tape format driver. |
| `C:` | Atari CIO | `VDCLASS_TAPE` | Atari Program Recorder 128-byte FSK audio cassette stream. |
| `MAG1:` - `MAG8:` | Magnetic Tape | `VDCLASS_TAPE` | General magnetic tape stream transport. |
| `MDM1:` - `MDM8:` | Tandy / TRS-80 | `VDCLASS_MODEM` | Tandy Hayes-compatible acoustic coupler / modem emulator. |
| `TEL1:` - `TEL8:` | Tandy Model 100 | `VDCLASS_CUSTOM` | Tandy auto-dialer and telephony interface peripheral. |
| `LCD1:` - `LCD8:` | Tandy Portable | `VDCLASS_DISPLAY` | Tandy multi-line liquid crystal character display driver. |
| `BAR1:` - `BAR8:` | Tandy TRS-80 | `VDCLASS_HID` | Tandy optical barcode / optical wand reader stream. |
| `WND1:` - `WND8:` | Tandy TRS-80 | `VDCLASS_HID` | Tandy optical barcode wand scanner device. |
| `LP1:` - `LP8:` | Tandy / TRS-80 | `VDCLASS_PRINTER` | Tandy Line Printer character stream. |
| `FDD1:` - `FDD8:` | Tandy Portable Disk | `VDCLASS_STORAGE` | Tandy Portable Disk Drive (100k/200k sector format). |
| `KBD1:` - `KBD8:` | Programmable HID | `VDCLASS_KEYMAP` | Programmable controller keymap matrices (QWERTY, Dvorak). |
| `SCR1:` - `SCR8:` | BGI / Raster | `VDCLASS_FRAMEBUFFER`| Off-screen 4-bit / 8-bit graphics framebuffers and HUD canvases. |
| `LPT1:` - `LPT8:` | MS-DOS / PC-DOS | `VDCLASS_PRINTER` | Parallel line printer spooler ports 1-8. |
| `PRN1:` - `PRN8:` | MS-DOS / PC-DOS | `VDCLASS_PRINTER` | Standard printer output channels. |
| `LST1:` - `LST8:` | CP/M / MBASIC | `VDCLASS_PRINTER` | Listing printer channels. |
| `PDF1:` - `PDF8:` | Vector Graphics | `VDCLASS_PRINTER` | Direct text-to-PDF document generator with multi-channel output. |
| `COM1:` - `COM8:` | RS-232 / UART | `VDCLASS_SERIAL` | Asynchronous serial communications ports with baud/parity config. |
| `AUX1:` - `AUX8:` | MS-DOS / CP/M | `VDCLASS_SERIAL` | Auxiliary serial communications ports. |
| `E:` | Atari CIO | `VDCLASS_CONSOLE` | Atari 8-bit Screen Editor (full-screen text window). |
| `K:` | Atari CIO | `VDCLASS_HID` | Atari Raw Keyboard stream. |
| `S:` | Atari CIO | `VDCLASS_DISPLAY` | Atari Graphics/Text screen drawing surface. |
| `D1:` - `D8:` | Atari CIO | `VDCLASS_STORAGE` | Atari multi-disk drive channels. |
| `P:` | Atari CIO | `VDCLASS_PRINTER` | Atari Line Printer stream. |
| `R1:` - `R8:` | Atari CIO | `VDCLASS_SERIAL` | Atari 850 RS-232 serial interfaces. |
| `V:` | Atari CIO | `VDCLASS_CUSTOM` | Atari Voice Synthesizer stream. |
| `PUN:` / `PTP:` | CP/M 2.2 / MP/M | `VDCLASS_STORAGE` | CP/M Paper Tape Punch stream (8-level punched tape). |
| `RDR:` / `PTR:` | CP/M 2.2 / MP/M | `VDCLASS_STORAGE` | CP/M Paper Tape Reader stream (XON/XOFF flow control). |
| `AXI:` / `AXO:` | CP/M 2.2 / MP/M | `VDCLASS_SERIAL` | CP/M Auxiliary Input and Auxiliary Output streams. |

---

## 3. Programmatic Device Introspection Functions

BASIC++ provides built-in functions allowing programs to discover and query registered devices dynamically at runtime:

### Introspection Functions

- **`DEVICES$`**: Returns the complete list of registered virtual devices as a newline-delimited string suitable for programmatic inspection.
- **`DEVSTATUS$(slot)`**: Returns the operational status of the device in slot number `slot` as an uppercase string: `"ACTIVE"`, `"IDLE"`, `"DISABLED"`, or `"EMPTY"`.
- **`DEVNAME$(slot)`**: Returns the unique registered name of the device in slot number `slot` (e.g. `"Console"`, `"VirtFS"`, `"Network"`).
- **`DEVTYPE$(slot)`**: Returns the class or type identifier string of the device in slot number `slot` (e.g. `"VCon"`, `"File"`, `"VNet"`, `"BGI"`).

### Dynamic Introspection Example

```basic
10 FOR S = 0 TO 15
20   T$ = DEVTYPE$(S)
30   IF T$ <> "" AND T$ <> "EMPTY" THEN
40     PRINT "Slot "; S; ": "; DEVNAME$(S); " ["; T$; "]"
45     PRINT "        Status="; DEVSTATUS$(S)
50   END IF
60 NEXT S
```

### Feature Detection with Defensive Blocks

Subsystem availability can also be tested defensively using structured exception handling:

```basic
10 TRY
20   SCREEN 12
30   GraphicsReady = 1
40 CATCH
50   GraphicsReady = 0
60   PRINT "Graphics unavailable, continuing in text mode."
70 END TRY
```

---

## 4. Device Management Statements

Virtual devices are managed interactively or programmatically via the `DEVICE` and `DEVICES` statements:

### Statement Syntax

```basic
DEVICES                                         ' Display all registered devices and aliases
DEVICE "dev_name:", "config_param=val"          ' Configure device parameters
DEVICE ALIAS "alias:" AS "target:"              ' Create user-defined device alias
DEVICE UNALIAS "alias:"                         ' Remove a device alias
DEVICE MOUNT "dev:" TYPE "driver" [OPTIONS "o"] ' Mount dynamic device driver
DEVICE RESET                                    ' Reset device manager to defaults
```

### Practical Device Command Examples

```basic
10 REM List all active devices
20 DEVICES
30 REM Create an alias mapping COM1 to standard serial port
40 DEVICE ALIAS "MODEM:" AS "COM1:"
50 OPEN "MODEM:9600,N,8,1" FOR RANDOM AS #1
60 CLOSE #1
70 DEVICE UNALIAS "MODEM:"
```
