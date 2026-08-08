# BASIC++ Universal Cross-Platform Portability & Environment Guide

## Architecture Overview
BASIC++ is engineered from the ground up to target **any operating system, any CPU architecture, and any runtime environment** — from 8-bit/32-bit bare-metal microcontrollers to 64-bit cloud clusters and WebAssembly in browser sandboxes.

---

## 1. Environment Editions & Target Binary Matrix

| Environment Edition | Code / Target Name | Binary Output | Primary OS & Targets | Default Heap | SDL2 Dependency |
|---|---|---|---|---|---|
| **Desktop Workstation** | `basicpp` | `basicpp.dll` / `libbasicpp.so` / `libbasicpp.dylib` | Windows, Linux, macOS | 640 MB | Delay-Loaded |
| **Cloud Server** | `baspp` | `baspp.exe` / `baspp` | Headless Linux, CGI, Docker | 256 MB | None (`NO_SDL2`) |
| **Internet of BASIC (IoB)** | `iob` | `iob.dll` / `libiob.so` | Raspberry Pi, Jetson, OpenWrt, ESP32 | 128 MB–384 MB | None (`NO_SDL2`) |
| **Web Application (WAP)** | `wap` | `wap.wasm` / `wap.js` | Web Browsers, Node.js, PWAs | 16 MB–64 MB | None (`WASM_BUILD`) |
| **Batch Script Runner** | `bs` | `bs.exe` / `bs` | PowerShell, Bash, Automated Jobs | 64 MB | None (`NO_SDL2`) |
| **Lite REPL** | `bpp` | `bpp.exe` / `bpp` | IoT Terminals, Serial Consoles | 384 MB | None (`NO_SDL2`) |
| **Embedded MCU** | `embedded` | `basstub.c` | RP2040, ESP32, STM32, Arduino | 2 KB–16 MB | None (`BASIC_EMBEDDED`) |
| **Mobile App** | `mobile` | `libbasicpp_mobile.so` / `.framework` | Android (JNI), iOS (Swift/Obj-C) | 128 MB | Native Bridge |

---

## 2. Multi-Language FFI Binding Matrix

| Language | Package Location | Supported Environments & Binaries | Interface Mechanism |
|---|---|---|---|
| **C / C++ (C17)** | `engine/include/bpp_api.h` | All (`basicpp.dll`, `libbasicpp.so`, `libbasicpp.dylib`, `libiob.so`) | Direct C17 Header |
| **Python 3** | `bindings/python/basicpp` | Windows, Linux, macOS (`basicpp`, `iob`) | `ctypes` FFI |
| **Rust** | `bindings/rust/basicpp-sys` | Windows, Linux, macOS | Cargo FFI sys-crate |
| **JavaScript / TypeScript** | `bindings/wasm` | Browser, Node.js (`wap.wasm`) | Emscripten WASM Bridge |
| **Go** | `bindings/go/basicpp` | Linux, Windows, macOS | `cgo` |
| **Java / Kotlin** | `engine/src/bootstrap/mobile/` | Android ARM64/x86_64 | JNI Bridge |
| **Swift / Obj-C** | `engine/src/bootstrap/mobile/` | iOS Devices & Simulators | C Module Map |

---

## 3. Environment Specific Guides
For detailed setup instructions, code snippets, and build flags for each environment profile, refer to:
- [Desktop Workstation Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/desktop.md) (`basicpp.dll` / `libbasicpp.so` / `libbasicpp.dylib`)
- [Cloud Server Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/server.md) (`baspp` / `bs`)
- [Internet of BASIC Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/iot.md) (`iob` / `libiob.so`)
- [Web Application BASIC Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/wasm.md) (`wap` / `wap.wasm`)
- [Bare-Metal Embedded MCU Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/embedded.md) (`basstub.c`)
- [Mobile Platforms Guide](file:///c:/Users/rtdos/GitHub/basic-plus-plus/docs/environments/mobile.md) (Android & iOS)
