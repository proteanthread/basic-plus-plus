# Bare-Metal Embedded MCU Edition (`basstub.c`)

## Overview
The **Embedded MCU Edition** compiles BASIC++ onto bare-metal microcontrollers with zero OS dependencies, static Flash ROM payload buffers, and configurable memory pools (2 KB–16 MB).

## Target Microcontrollers
- **RP2040**: Raspberry Pi Pico / Pico W
- **ESP8266 & ESP32**: ESP-IDF / FreeRTOS
- **STM32**: Nucleo, Discovery, Custom STM32F4/F7/H7 board targets
- **Arduino**: Portenta H7, Nano 33 BLE, MKR series

## Memory & C17 Architecture
- **Zero Dynamic Allocator Requirement**: Can run using static Flash ROM bytecode buffers
- **Weak Selectany Linker Pattern**: Payload symbols (`g_embedded_bytecode`) use weak linkage allowing transpiled C code overrides
