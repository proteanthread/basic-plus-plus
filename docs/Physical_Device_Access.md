<!--
Title:        Physical_Device_Access
Tier:         1
Applies to:   BASIC++ v6.5.2, hosted targets
Authority:    engine/lib/platform/plat_serial.c, plat_console.c,
              engine/src/statements/io/gpib.c,
              engine/src/statements/system/hardware/,
              engine/src/statements/system/mouse.c, joystick.c, pen.c,
              engine/src/eval/functions/system/hardware/,
              engine/include/basicpp_sys.h
Generated:    no, hand-written
Status:       current
-->

# Physical Device Access

What BASIC++ can actually touch on real hardware, and what it only appears to.

`Device_Reference` and `Virtual_Devices` describe the **virtual** device
catalog — the 29 device classes, the `CON:`, `LPT:`, `CAS:` prefixes and the
rest. This document answers the different question: **which of those reach a
physical device, and which do not?**

The short answer: serial ports, printers, the console, port and memory I/O on
x86, and the GPIB instrument bus are real. Every pointing and game input device
is registered and empty. USB, CD-ROM, CD audio and DVD do not exist at all.

---

## 1. Real: serial ports

`engine/lib/platform/plat_serial.c` is genuine hardware access, on both hosts.

**Windows.** `CreateFileA` on `\\.\COMn` with `GENERIC_READ | GENERIC_WRITE`,
then a `DCB` configured through `SetCommState` for baud, data bits, parity and
stop bits, `COMMTIMEOUTS` for read and write timeouts, and `ReadFile` and
`WriteFile` for transfer.

**POSIX.** `open` on the tty device with `O_RDWR | O_NOCTTY | O_NONBLOCK`, a
`termios` struct configured and applied with `tcsetattr(TCSANOW)`, and
`ioctl(FIONREAD)` to find out how many bytes are waiting.

From BASIC this is reached through `OPEN "COM1:..." AS #n` and the `BAUD`
statement and `BAUD` function. A program can drive a modem, a serial printer,
a microcontroller's USB-serial bridge, a GPS receiver, a barcode scanner or an
industrial sensor, and that is the most useful physical capability the language
currently has.

```basic
10 OPEN "COM3:9600,N,8,1" AS #1
20 PRINT #1, "AT"
30 LINE INPUT #1, Reply$
40 PRINT "Modem said: "; Reply$
50 CLOSE #1
```

Note the practical consequence: a USB device that presents a serial port —
which most microcontroller boards, many instruments and nearly all GPS
receivers do — **is** reachable, even though there is no USB support. The
operating system's serial driver does the work.

---

## 2. Real: the instrument bus

`engine/src/statements/io/gpib.c` implements GPIB / IEEE-488 statements. This
is the bus that laboratory instruments — oscilloscopes, signal generators,
multimeters, spectrum analysers — have used since 1975, and having it in a
BASIC is historically apt: HP BASIC on the 9000 series was the standard
instrument-control language for two decades.

---

## 3. Real: ports, memory and interrupts on x86

| Statement or function | Purpose |
|---|---|
| `OUT port, data` | Write a byte to an I/O port. Validates 0-65535 |
| `INP(port)` | Read a byte from an I/O port |
| `POKE addr, value`, `POKEB` | Write to memory |
| `PEEK(addr)` | Read from memory |
| `DEF SEG`, `DEF USR` | Segment and user-routine addressing |
| `INTERRUPT` | Invoke a software interrupt |
| `WAIT port, mask [, xor]` | Block until a port matches |
| `RBYTE`, `WBYTE` | Byte-level device transfer |
| `PORT` | Port configuration |

These call `sys_in8`, `sys_out8`, `sys_in16` and `sys_out16` in
`engine/include/basicpp_sys.h`. Three limits matter and are documented here
rather than discovered:

1. **8- and 16-bit only.** There is no `sys_in32` or `sys_out32`.
2. **x86 and x86_64 only.** On any other architecture the port functions
   compile to a no-op that returns 0, **with no diagnostic**. A program that
   drives hardware through `OUT` on an ARM or RISC-V board will build, run,
   and do nothing.
3. **`sys_memory_barrier()` is a compiler barrier, not a CPU fence.** It does
   not order MMIO against DMA on a weakly ordered architecture.

Items 2 and 3 are correctness bugs, not missing features, and are listed in
the 7.0.0 plan as phase 3.4.

---

## 4. Real: console and printers

`plat_console.c` (19 KB) is the real terminal layer: raw and cooked modes,
cursor positioning, colour attributes, key reading, window size. `INKEY$`,
`LOCATE`, `COLOR`, `CLS`, `WIDTH` and `SCREEN 0` all reach it.

`vprinter.c` and `pdf_writer.c` back the `LPT:` and `PRN:` device families;
`LPRINT` and `LLIST` route through them. Output can go to a real printer via
the host spooler, or to a PDF.

The keyboard is the one human-input device that works completely.

---

## 5. Stubbed: every other human-input device

This is the finding a reader most needs, because these statements are
registered, they parse, and they return success.

| Surface | Source | State |
|---|---|---|
| `MOUSE`, `MOUSE INPUT`, `MOUSE SHOW`, `MOUSE HIDE` | `statements/system/mouse.c` | All handlers are `(void)vm; (void)lex; return err;` |
| `HMOUSE`, `VMOUSE` | same file | Same |
| `PEN` | `statements/system/pen.c` | Handler empty |
| `STICK`, `STRIG` statements | `statements/system/joystick.c` | Handler empty |
| `STICK(port%)` function | `eval/functions/system/hardware/stick.c` | Casts `port` to void, returns 0 |
| `STRIG(n)`, `PADDLE(n)`, `PTRIG(n)` | same directory | Same shape |

`STICK(port%)`'s own registered description says it "autodetects USB
controller or falls back to cursor keys". It does neither. That description is
a defect and is listed as such in `Implementation_Status`.

The practical effect: **a BASIC++ program cannot read a mouse, a lightpen, a
joystick or a paddle.** A game or a TUI that wants pointer input has to be
driven from the keyboard, or from a C host through `basicpp.h`.

The pieces needed to fix this are already present — `plat_console.c` for the
terminal path and `hal_sdl2.c` for the graphical one — which is why the 7.0.0
plan puts it in phase 3.7 rather than treating it as new work.

---

## 6. Absent entirely

| Capability | State |
|---|---|
| USB (as a bus) | No source. See section 1: USB-serial devices work through the serial layer |
| CD-ROM data | No source, no ISO9660 driver, no block device layer |
| CD audio | No source. No `CDPLAY`, no MSCDEX-style interface |
| DVD | No source |
| SCSI, IDE, NVMe direct access | No source. There is no block device layer beneath the VFS at all |
| Camera | `CAP_CAMERA` exists as a module capability flag; nothing implements it |
| Scanner, MIDI, gamepad HID | No source |

The block device layer is the prerequisite for most of these and is phase 4.1
of the 7.0.0 plan. Until it exists, "read a sector from a CD" has nowhere to
land.

---

## 7. Simulated hardware, which is a different thing again

Some devices are neither real nor absent: they are simulated, deliberately, so
that a program can be developed without the hardware present.

| Family | Backing | Note |
|---|---|---|
| ESP32 GPIO, I2C, SPI, PWM, ADC | `engine/iot/src/esp32_hal.c` | `s_sim_pin_*` arrays; I2C is a 256x256 byte matrix; SPI is a loopback |
| DHT, touch, Hall, NeoPixel | `iot_sensors.c` | Fixed readings: 24.5 C, 45%, 32, 128 |
| WiFi, Bluetooth, BLE, ESP-NOW, MQTT | `iot_net.c` | Flags and invented strings |
| Cassette, tape containers, modem, barcode wand | `dev_tape.c`, `dev_tandy.c`, `dev_atari.c` | Vintage peripheral personalities over files |

The vintage peripheral emulations in the last row are honestly simulated —
they exist to let a TRS-80 or Atari program run on a modern machine, and there
is no physical device to reach. The ESP32 and radio rows are placeholders
waiting for real implementations; see `IoT_And_Embedded_Guide`.

---

## 8. How to reach a device BASIC++ does not support

Three routes, in order of how much work they are.

**Through a serial port.** If the device speaks serial, or presents a
USB-serial bridge, section 1 already covers it and no new code is needed.

**Through a C host.** Embed the engine with `basicpp.h`, do the device work in
C where you have the whole operating system available, and expose it to BASIC
as a message on the bus or a variable. See `C_Programmers_Guide`.

**Through a user-defined virtual device.** `engine/include/device/vdev.h`
supports `UserDefinedDevice`, whose operations are BASIC subroutines reached by
`GOSUB`. That lets a device be implemented in BASIC itself, driving whatever
lower-level access is available. It is the slowest of the three and the most
self-contained. See `Virtual_Devices` and `Extension_Guide`.

---

## See also

- `Device_Reference` for the full virtual device catalog
- `Virtual_Devices` for the device model and user-defined devices
- `Implementation_Status` for the evidence register
- `Systems_Programming` for `PEEK`, `POKE`, `INP`, `OUT` in context
- `IoT_And_Embedded_Guide` for the ESP32 simulation
- `C_Programmers_Guide` for the embedding route
