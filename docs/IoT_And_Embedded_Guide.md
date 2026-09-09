<!--
Title:        IoT_And_Embedded_Guide
Tier:         1
Applies to:   BASIC++ v6.5.2, targets bpp and iot
Authority:    engine/iot/, engine/iot/CMakeLists.txt, engine/src/statements/,
              engine/src/eval/functions/system/hardware/
Generated:    no, hand-written
Status:       current
-->

# IoT and Embedded Guide

## Read this first

BASIC++ has a complete IoT programming surface: twenty statements and six
functions covering pins, analogue I/O, buses, sensors, timing, power and
wireless. You can write, run and debug IoT programs with it today.

**It runs on your desktop, not on a microcontroller.**

`engine/iot/` is a simulator. Every hardware call writes to or reads from a
static array in host memory. `esp32_hal_digital_write()` sets a byte in
`s_sim_pin_vals[40]`; it does not touch a GPIO register. `esp32_hal_delay_ms()`
calls Windows `Sleep()` or POSIX `nanosleep()`. `iot_main.c` has an ordinary
`int main(int argc, char **argv)`, not an ESP-IDF `app_main()`, and there is no
FreeRTOS integration, no linker script, and no flashing step anywhere in the
build.

That is stated here at the top because it determines what this guide can
honestly teach you. It teaches you to write IoT programs and verify their
logic. It does not teach you to flash an ESP32, because BASIC++ cannot yet
produce firmware for one. Section 12 states exactly what is missing and what
it would take.

What the simulator is genuinely good for: developing the program logic, the
control flow, the timing structure and the protocol handling on a machine with
a real debugger and no wiring, before any hardware exists. That is a normal and
useful stage of embedded work, and it is what you have.

---

## 1. What actually exists

Measured from `engine/iot/`, which is a self-contained sub-project with its own
`CMakeLists.txt` building a static library called `libiot`.

| File | Size | What it does |
|---|---|---|
| `include/esp32_hal.h` | 1,312 | Pin, analogue, bus and timing interface |
| `include/esp32_regs.h` | 1,313 | Real ESP32 register addresses, emulated access |
| `include/esp32_serial.h` | 1,187 | Serial console interface |
| `include/iot_net.h` | 2,165 | WiFi, MQTT, ESP-NOW, Bluetooth, NFC interface |
| `include/iot_runtime.h` | 676 | Device profile configuration |
| `include/iot_sensors.h` | 949 | NeoPixel, DHT, touch, Hall interface |
| `src/esp32_hal.c` | 4,305 | Simulated pin state in static arrays |
| `src/esp32_regs.c` | 3,683 | Emulated register bank |
| `src/esp32_serial.c` | 5,694 | Serial console |
| `src/iot_net.c` | 5,388 | Network stubs |
| `src/iot_sensors.c` | 1,516 | Sensor drivers returning fixed values |
| `src/iot_main.c` | 9,284 | Entry point for the `iot` binary |

`libiot` is linked into `libkernel` at line 222 of `engine/CMakeLists.txt`, so
the IoT statements are available in the desktop builds as well as in the
dedicated `iot` binary.

### Two binaries are called iot, and you probably want the second

This trips people up and it is worth being explicit.

- **`bpp`** is built from `engine/src/bootstrap/iot/iot.c` with the headless
  object set. It is the lightweight REPL edition.
- **`iot`** is built from `engine/iot/src/iot_main.c` and links `libiot`
  and `libscript`. This is the IoT edition, and it is the one that carries
  the device profile and the hardware statements.

Both are defined in the root `CMakeLists.txt`, at lines 113 and 193
respectively. The naming collision is a known defect; it is recorded rather
than worked around.

---

## 2. The statement and function surface

Every entry below is a real registered keyword with its own reference page
under `docs/keywords/`. This guide explains how they fit together; the
per-keyword pages carry the exact syntax, error codes and descriptor fields.

**Pins and digital I/O** — `PINMODE`, `DWRITE`, `DREAD`

**Analogue** — `AREAD`, `DAC`, `PWM`, `SERVO`

**Buses** — `I2C`, `SPI`

**Sensors and light** — `DHT`, `NEOPIXEL`, `TOUCH`, `HALL`

**Timing and power** — `DELAY`, `EVERY`, `TICKS`, `DEEPSLEEP`, `FREQ`,
`REBOOT`

**Memory** — `MEM`

**Wireless and network** — `WIFI`, `MQTT`, `ESPNOW`, `BT`, `NFC`, `WEBREPL`

---

## 3. Your first program

Blink an LED on pin 2, which is the on-board LED position on most WROOM-32
development boards.

```basic
10 PINMODE 2, OUTPUT
20 FOR I = 1 TO 10
30   DWRITE 2, 1
40   DELAY 500
50   DWRITE 2, 0
60   DELAY 500
70 NEXT I
80 PRINT "Done."
```

Run it with the IoT binary:

```
iot blink.bas
```

Under the simulator, `DWRITE` sets `s_sim_pin_vals[2]` and `DELAY` sleeps the
host process for the requested milliseconds. The timing is real; the pin is
not.

---

## 4. Pins and digital I/O

The simulator models **40 pins, numbered 0 to 39**, matching the ESP32 GPIO
range. Any pin number outside that range is silently ignored rather than
raising an error, which is worth knowing when a program appears to do nothing.

`PINMODE pin, mode` sets the direction. The four modes are defined in
`esp32_hal.h`:

| Mode | Value | Meaning |
|---|---|---|
| `INPUT` | 0 | High impedance input |
| `OUTPUT` | 1 | Push-pull output |
| `INPUT_PULLUP` | 2 | Input with internal pull-up |
| `INPUT_PULLDOWN` | 3 | Input with internal pull-down |

`DWRITE pin, value` writes a digital level; any non-zero value becomes 1.
`DREAD(pin)` reads it back.

In the simulator the pull-up and pull-down modes are recorded but have no
effect on what `DREAD` returns, because there is no external circuit to pull
against. A pin reads back whatever was last written to it.

---

## 5. Analogue output and input

`AREAD(pin)` returns a 12-bit reading, 0 to 4095, matching the ESP32 ADC
resolution.

Its simulator behaviour is specific and you should know it before you write a
sensor-reading loop: if nothing has set an analogue value for the pin and the
pin's digital value is non-zero, `AREAD` returns **4095**. Otherwise it returns
the stored analogue value, which starts at 0. So a pin you have driven high
reads as full scale, and an untouched pin reads as zero. There is no noise, no
drift and no non-linearity, which means analogue code that works here has not
been tested against anything an ADC actually does.

`DAC pin, value` takes 0 to 255 and clamps to that range.

`PWM pin, frequency, duty` records a frequency and duty cycle. Nothing
oscillates; the values are stored and can be read back by the runtime.

`SERVO pin, angle` clamps to 0 to 180 degrees.

---

## 6. Buses

`I2C` reads and writes a simulated device bank: a 256 by 256 byte array
indexed by device address and register. Writing to address 0x68 register 0x00
and reading it back returns what you wrote. This is enough to exercise a
driver's register sequencing and completely insufficient to test against a real
device, which will have timing requirements, NAKs, clock stretching and
registers that change on their own.

`SPI` is a **loopback**. `esp32_hal_spi_transfer()` copies the transmit buffer
into the receive buffer byte for byte and returns true. Whatever you send is
what you get back.

---

## 7. Sensors

These return fixed values. Every program that reads them will see the same
numbers on every run, which is deterministic and useful for testing control
logic, and useless for testing anything that depends on the readings changing.

| Call | Returns | Source |
|---|---|---|
| `DHT pin, t, h` | 24.5 degrees C, 45.0 % | `iot_dht_read()` |
| `TOUCH(pin)` | 32 | `iot_touch_read()` |
| `HALL` | 128 | `iot_hall_read()` |

`NEOPIXEL` is different: it maintains a real pixel buffer of up to 256 RGB
triples per pin, so setting and clearing pixels genuinely updates state you can
inspect. `iot_neopixel_show()` is empty, because there is no strip to latch the
data out to.

---

## 8. Timing and power

`DELAY ms` and the microsecond variant map to host sleeps. On Windows the
microsecond delay is implemented with `Sleep()` at a one-millisecond floor, so
sub-millisecond timing is not achievable there.

`TICKS` returns a monotonic count from `clock_gettime(CLOCK_MONOTONIC)` on
POSIX and `GetTickCount64()` or `QueryPerformanceCounter()` on Windows. These
are genuine high-resolution host timers, so elapsed-time measurement in your
program is real.

`EVERY` schedules periodic execution. `DEEPSLEEP`, `FREQ` and `REBOOT` are
power and clock control statements whose simulator behaviour does not change
the host machine's state.

---

## 9. Memory

The IoT profile defaults to **2 MB**, set in `iot_main.c`:

```c
static IotProfileConfig s_active_config = {
    .device_name = "Generic",
    .ram_size_bytes = 2097152L,   // 2 MB Default
    .is_target_configured = false,
    .auto_run_enabled = true
};
```

`MEM` reports available memory against that profile. Note
`is_target_configured` defaults to false: no specific board has been selected,
and the profile is generic until one is.

---

## 10. Wireless and network

`WIFI`, `MQTT`, `ESPNOW`, `BT`, `NFC` and `WEBREPL` are declared in
`iot_net.h` and implemented in `iot_net.c`. Verify the behaviour of each
against that file before relying on it in a program; the sensor pattern above,
where a plausible interface returns a fixed value, appears elsewhere in this
subsystem too.

Note also that there is **no TLS anywhere in BASIC++**. Any protocol that
requires transport security cannot be genuinely implemented today, whatever an
interface suggests.

---

## 11. Building

The IoT edition is built by the root `CMakeLists.txt` as the `iot` target:

```
add_executable(iot engine/iot/src/iot_main.c ...)
target_include_directories(iot PRIVATE engine/include engine/iot/include)
target_compile_definitions(iot PRIVATE BASIC_LITE_BUILD BASIC_HEADLESS_BUILD NO_SDL2)
target_link_libraries(iot PRIVATE libiot libscript ${SYS_LIBS})
```

So a normal desktop build produces it:

```
cmake -S . -B build_win
cmake --build build_win --target iot
```

On Windows use `tools/build_win.ps1`; on Linux `tools/build_linux.sh`.

`engine/iot/` can also be built on its own, because it carries its own
`CMakeLists.txt` declaring `project(basicpp_iot C)` and producing `libiot` as a
static library. It compiles under C17 with `-Wall -Wextra -Wpedantic -Werror`,
or `/W4 /WX` under MSVC.

---

## 12. What it would take to run on real silicon

This section exists so that nobody plans around a capability that is not
there. Everything below is **not implemented**.

**The HAL would need a real implementation.** `esp32_hal.c` is the simulator.
A hardware build needs a second implementation of the same interface writing to
the register addresses already defined in `esp32_regs.h` — `GPIO_OUT_W1TS_REG`
at `0x3FF44004`, `GPIO_OUT_W1TC_REG` at `0x3FF44008`, `GPIO_IN_REG` at
`0x3FF4403C`. Those addresses are correct and come from the ESP32 Technical
Reference Manual, which is a genuine head start: the interface and the register
map both exist, only the implementation joining them is missing.

**The host dependencies would have to go.** `esp32_hal.c` includes
`<windows.h>` or `<unistd.h>` and `<time.h>`, and the sub-project's CMake sets
`_POSIX_C_SOURCE`, `_XOPEN_SOURCE` and `_GNU_SOURCE` on UNIX. None of that
exists on a microcontroller. Delays would come from a hardware timer and ticks
from the RTC or a systick counter.

**There would need to be a firmware entry point.** `iot_main.c` has
`int main(int argc, char **argv)` and parses a command line. ESP-IDF expects
`app_main()`; a bare-metal build expects a reset handler and a linker script.
Neither exists.

**The toolchain and flashing step do not exist.** No Xtensa or RISC-V
toolchain is configured, no ESP-IDF component manifest, no partition table, no
`idf.py flash` or `esptool` invocation anywhere in the build.

**The sensor drivers are stubs.** DHT needs a bit-banged single-wire protocol
with microsecond timing. NeoPixel needs an 800 kHz WS2812 waveform, usually via
RMT or I2S. Neither is written.

**Memory would need re-profiling.** The 2 MB default is a desktop figure. A
WROOM-32 has 520 KB of internal SRAM, and the Embedded profile in
`engine/include/types/config.h` is the relevant one.

That is the honest list. The interfaces, the register map and the whole BASIC
language surface are already in place, which is the hard part of the design.
What is absent is the platform layer underneath them.

---

## 13. Where the code is

| Concern | Path |
|---|---|
| HAL interface | `engine/iot/include/esp32_hal.h` |
| HAL simulator | `engine/iot/src/esp32_hal.c` |
| Register map | `engine/iot/include/esp32_regs.h` |
| Sensors | `engine/iot/src/iot_sensors.c` |
| Networking | `engine/iot/src/iot_net.c` |
| Serial console | `engine/iot/src/esp32_serial.c` |
| Entry point | `engine/iot/src/iot_main.c` |
| Sub-project build | `engine/iot/CMakeLists.txt` |
| Executable target | root `CMakeLists.txt`, line 193 |
| Statement handlers | `engine/src/statements/` |
| Function handlers | `engine/src/eval/functions/system/hardware/` |

## See also

- `Embedded_Platforms.md` for the wider platform picture
- `Memory_Maps.md` for the memory profiles
- `Systems_Programming.md` for `PEEK`, `POKE`, `INP` and `OUT`
- The per-keyword pages under `docs/keywords/statements/` and
  `docs/keywords/functions/`
