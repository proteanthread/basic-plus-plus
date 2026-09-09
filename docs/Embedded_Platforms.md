<!--
Title:        Embedded_Platforms
Tier:         2
Applies to:   BASIC++ v6.5.2 (iot, embedded targets)
Authority:    engine/src/platform/embedded/, iot.c, cmake/
Generated:    no, manual specification
Status:       current
-->

# BASIC++ v6.5.2 Embedded Platforms

## 1. OVERVIEW

BASIC++ can be compiled for microcontroller and embedded systems using the `BASIC_EMBEDDED` profile. This profile targets devices with as little as 32 KB RAM and 128 KB flash, producing a minimal, deterministic BASIC execution engine suitable for IoT applications, robotics, sensor logging, and industrial controllers.

## 2. SUPPORTED TARGETS

| Platform | MCU Architecture | RAM | Flash | Profile Status |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32** | Xtensa Dual-Core LX6 | 520 KB | 4 MB | Supported (`iot`) |
| **Raspberry Pi Pico** | ARM Cortex-M0+ Dual-Core | 264 KB | 2 MB | Supported (`iot`) |
| **STM32F4** | ARM Cortex-M4 | 192 KB | 1 MB | Supported (`iot`) |
| **Arduino Due** | ARM Cortex-M3 | 96 KB | 512 KB | Supported (`iot`) |
| **Arduino Mega** | ATmega2560 (8-bit) | 8 KB | 256 KB | Experimental Subset |

## 3. MEMORY PROFILE

Under the minimal embedded profile, memory partitions are scaled for constrained hardware:

| Memory Region | Default Allocation |
| :--- | :--- |
| **Program Memory** | 8 KB |
| **Variable Memory** | 4 KB |
| **String Heap** | 4 KB |
| **Scratch Arena** | 2 KB |

Limits under minimal profile: Stack depth 31; Named variables 64; DIM arrays 16; Maximum array elements 512; User-defined functions 8. The baseline runtime footprint is approximately 18 KB RAM, preserving the remainder of device memory for network buffers, hardware queues, and RTOS stacks.

## 4. BUILD CONFIGURATION

Cross-compilation uses platform-specific CMake toolchain files:

```bash
# ESP32 cross-compilation
mkdir build_esp32 && cd build_esp32
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/esp32.cmake -DBASIC_EMBEDDED=ON
cmake --build .

# Raspberry Pi Pico cross-compilation
mkdir build_pico && cd build_pico
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/pico.cmake -DBASIC_EMBEDDED=ON
cmake --build .
```

## 5. FEATURE GATE DEFAULTS

The embedded profile suppresses desktop and hosted dependencies:
- **Disabled Subsystems**: SDL2 GUI, BGI desktop rasterizer, desktop audio, segmented memory (`vmem`), TUI multiplexer, and dynamically loaded modules.
- **Active Builtin Features**: Core linear VM and AST evaluator, serial stream console (`PRINT`, `INPUT`), arithmetic expressions, structured control flow (`FOR..NEXT`, `WHILE..WEND`, `DO..LOOP`, `IF..THEN..ELSE`), `SUB` and `FUNCTION` definitions, `ON ERROR GOTO` error trapping, direct hardware manipulation (`PEEK`, `POKE`), and GPIO pin mapping (`INP`, `OUT`).

## 6. HARDWARE ABSTRACTION

The embedded platform layer maps standard BASIC++ I/O statements to hardware peripherals:
- `PRINT`: Transmits characters over the active UART/serial interface.
- `INPUT`: Reads incoming characters from UART/serial input buffers.
- `POKE address, value`: Writes directly to memory-mapped peripheral registers.
- `PEEK(address)`: Reads values from memory-mapped hardware registers.
- `INP(port)` and `OUT port, value`: Access GPIO pins directly via port abstraction (ports 0-31 correspond to physical GPIO pins).

```basic
10 OUT 13, 1         ' Set GPIO 13 HIGH (LED on)
20 SLEEP 1
30 OUT 13, 0         ' Set GPIO 13 LOW (LED off)
40 SLEEP 1
50 GOTO 10
```

## 7. SERIAL CONSOLE & IOT EDITION

The dedicated `iot.exe` / `iot` executable target is engineered specifically for embedded microcontrollers. It provides a headless micro-REPL with an Apple II / Commodore style `]` prompt and `Ready.` status:

```
BASIC++ IoT Edition v6.5.2
18 KB RAM Available.

Ready.
] PRINT "Hello from Pico!"
Hello from Pico!
Ready.
]
```

## 8. PROGRAM STORAGE

For platforms lacking filesystem storage, programs can be persisted to onboard flash EEPROM sectors using `SAVE` and retrieved with `LOAD`. When an external SPI/SD card interface is attached with a FAT driver, standard file naming (`SAVE "LOGGER.BAS"`, `LOAD "LOGGER.BAS"`) operates through virtual file channels.

## 9. INCREMENTAL FEATURE SCALING

The embedded engine allows capabilities to be selectively enabled via CMake flags:
- `SUPPORT_ARRAYS=ON`: Enables multidimensional `DIM` and `REDIM`.
- `SUPPORT_FILE=ON`: Activates file system channels for attached SD cards.
- `SUPPORT_TIMER=ON`: Enables asynchronous hardware interval timer traps (`ON TIMER`).

## 10. REAL-TIME CONSTRAINTS & GC DISCIPLINE

BASIC++ provides deterministic execution, but does not guarantee hard real-time latency when string garbage collection triggers. For time-critical control loops, programs should pre-allocate string buffers using `STRING$` and avoid continuous string concatenation within high-frequency sensor loops.
