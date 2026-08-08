# IoB: Internet of BASIC (`iob` / `libiob.so` / `bpp`)

## Overview
**IoB (Internet of BASIC)** is the official IoT, Industrial Gateway, and Edge Computer edition of BASIC++. Designed for embedded Linux (Raspberry Pi, BeagleBone, Jetson, OpenWrt) and smart gateways.

## Binary Targets
- **`libiob.so` / `iob.dll`**: Shared C SDK for IoT gateways and Linux edge nodes
- **`bpp`**: Lite REPL Terminal Edition (`]` prompt, 384 MB default heap)

## Supported IoT & Hardware Targets
- Raspberry Pi (Zero, 3, 4, 5) / RPi Pico W
- NVIDIA Jetson Nano / Orin Nano
- BeagleBone Black / Green
- Industrial OpenWrt Routers & LoRaWAN Gateways
- Cellular M2M Modules (Quectel, Telit, u-blox)

## Peripheral & Bus Integration
- **UART/Serial VDev (`CON:`)**: Raw ASCII hardware serial binding
- **FujiNet Network Stack**: Virtual network channels (`vnet`) for WiFi/Ethernet telemetry
- **GPIO & Bit Manipulation**: Built-in bitwise operations (`SETBIT`, `CLRBIT`, `TOGGLEBIT`, `SHL`, `SHR`)
