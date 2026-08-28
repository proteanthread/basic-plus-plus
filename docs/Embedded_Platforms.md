# BASIC++ v6.5.2 Embedded Platforms

## 1. OVERVIEW

BASIC++ can be compiled for microcontroller and embedded platforms using the BASIC_EMBEDDED profile. This profile targets devices with as little as 32 KB RAM and 128 KB flash, producing a minimal BASIC interpreter suitable for IoT applications, educational hardware, and hobbyist projects.

## 2. SUPPORTED TARGETS

| Platform | MCU | RAM | Flash | Status |
|----------|-----|-----|-------|--------|
| ESP32 | Xtensa LX6 | 520 KB | 4 MB | Experimental |
| Raspberry Pi Pico | ARM Cortex-M0+ | 264 KB | 2 MB | Experimental |
| Arduino Mega | ATmega2560 | 8 KB | 256 KB | Experimental |
| STM32F4 | ARM Cortex-M4 | 192 KB | 1 MB | Experimental |
| Arduino Due | ARM Cortex-M3 | 96 KB | 512 KB | Experimental |

## 3. MEMORY PROFILE

| Region | Size |
|--------|------|
| Program Memory | 8 KB |
| Variable Memory | 4 KB |
| String Heap | 4 KB |
| Scratch Area | 2 KB |

Stack depths: 31 (all stacks). Named variable limit: 64. DIM arrays: 16. Array elements: 512. User-defined functions: 8.

The total BASIC memory footprint is approximately 18 KB, leaving remaining RAM for the platform SDK, stack, and hardware drivers.

## 4. BUILD CONFIGURATION

Cross-compilation uses the platform-specific CMake toolchain:

```bash
# ESP32
mkdir build_esp32
cd build_esp32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/esp32.cmake -DBASIC_EMBEDDED=ON
cmake --build .

# Raspberry Pi Pico
mkdir build_pico
cd build_pico
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/pico.cmake -DBASIC_EMBEDDED=ON
cmake --build .
```

## 5. FEATURE GATE DEFAULTS

The embedded profile disables:

- All graphics (SDL2, BGI rasterizer).
- Sound (no audio hardware).
- Networking (no TCP/IP stack; serial communication only).
- File I/O (no filesystem by default; optionally enabled with SD card).
- TUI editor (no terminal emulation).
- Module system (no dynamic loading).
- Segmented memory (vmem).
- Background tasks.
- Security system (single-user embedded context).
- Debugging (no DAP server).

Available features:
- PRINT (output to serial console).
- INPUT (read from serial console).
- Core arithmetic and string operations.
- FOR/NEXT, WHILE/WEND, IF/THEN/ELSE.
- SUB/FUNCTION (limited nesting).
- GOSUB/RETURN (31 levels).
- DEF FN (8 functions).
- ON ERROR GOTO (basic error handling).
- POKE/PEEK (direct hardware register access).
- INP/OUT (GPIO pin access through virtual port mapping).

## 6. HARDWARE ABSTRACTION

The embedded platform layer maps BASIC++ I/O operations to hardware:

**PRINT** — Sends text to UART/serial output.

**INPUT** — Reads from UART/serial input.

**POKE address, value** — Writes to a hardware register at the specified address. On ARM platforms, this is a memory-mapped I/O write.

**PEEK(address)** — Reads a hardware register.

**INP(port)** and **OUT port, value** — Map to GPIO pin read/write operations. Port numbers 0-31 correspond to GPIO pins 0-31.

```basic
10 OUT 13, 1         ' Set GPIO pin 13 HIGH (LED on)
20 SLEEP 1
30 OUT 13, 0         ' Set GPIO pin 13 LOW (LED off)
40 SLEEP 1
50 GOTO 10
```

## 7. SERIAL CONSOLE

The bpp lite edition is the recommended build for embedded targets. It provides the ]  prompt with Ready. status and a headless REPL that works over a serial connection:

```
BASIC++ Lite Edition v6.5.2
18 KB RAM Available.

Ready.
] PRINT "Hello, Pico!"
Hello, Pico!
Ready.
]
```

## 8. PROGRAM STORAGE

On embedded platforms without a filesystem, programs can be stored in flash memory. SAVE stores the program to a flash partition. LOAD reads it back. Only one program can be stored at a time unless an SD card is attached.

With an SD card: SAVE "PROG.BAS" and LOAD "PROG.BAS" work normally through the FAT filesystem driver.

## 9. ADDING FEATURES INCREMENTALLY

The embedded profile starts minimal and allows features to be enabled individually:

```cmake
set(BASIC_EMBEDDED ON)
set(SUPPORT_ARRAYS ON)       # Enable DIM/REDIM
set(SUPPORT_FILE ON)         # Enable file I/O (requires SD card)
set(SUPPORT_TIMER ON)        # Enable ON TIMER
```

Each enabled feature increases the code and RAM footprint. The Building A Minimal BASIC++ guide provides detailed sizes for each feature.

## 10. REAL-TIME CONSTRAINTS

BASIC++ on embedded platforms does not provide real-time guarantees. The garbage collector for the string heap runs synchronously and may cause brief pauses. For time-critical applications, minimize string operations in tight loops and pre-allocate strings with STRING$ before entering time-sensitive sections.
