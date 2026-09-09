<!--
Title:        Building BASIC++
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    CMakeLists.txt, engine/CMakeLists.txt
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Compilation and Build Guide

A comprehensive technical guide for configuring, compiling, and deploying all BASIC++ v6.5.2 binary targets across Windows and Linux systems.

## 1. Prerequisites and Toolchain Requirements

Building BASIC++ requires:
- **CMake 3.16 or later**: For build configuration and target generation.
- **ISO C17-Compliant C Compiler**: MSVC 2019+ (MSVC 19.28+), GCC 7+, or Clang 5+. Open Watcom is supported for 16-bit FreeDOS targets.
- **SDL2 Development Libraries** (optional, required only for the `baspp` desktop graphics edition): On Windows, bundled in `sdl2/`. On Linux, `libsdl2-dev`.
- **System Libraries**: On Windows, `ws2_32` and `winmm`. On Linux, `-lm` and `-lpthread`.

---

## 2. Standard Build Workflows

### 2.1 Windows (MSVC x64)

```powershell
cd basic-plus-plus
mkdir build_win
cd build_win
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --target baspp bpp bs iot bppc trans detok
```

### 2.2 Linux (GCC / Clang)

```bash
cd basic-plus-plus
mkdir build_linux
cd build_linux
cmake .. -DCMAKE_C_STANDARD=17 -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) baspp bpp bs iot bppc trans detok
```

---

## 3. Binary Build Targets & Memory Allocations

BASIC++ defines seven distinct executable binary targets:

| Target | Description | Upstream Link Layer | Default Memory Pool |
| :--- | :--- | :--- | :--- |
| `baspp` | Flagship Desktop Edition (Console + SDL2) | `libadvanced` | 640 MB (`671088640L`) |
| `bpp` | Lite Headless REPL Edition | `libcore` | 384 MB (`402653184L`) |
| `bs` | Batch Script Runner (PowerShell/Bash pipelines) | `libscript` | 64 MB (`67108864L`) |
| `iot` | Microcontroller & IoT micro-REPL | `libkernel` | 2 MB (`2097152L`) |
| `bppc` | Compiler & Bytecode Emitter | `libengine` | Dynamic |
| `trans` | Source-to-Source ISO C17 Transpiler | `libengine` | Dynamic |
| `detok` | GW-BASIC Binary File Detokenizer | `libkernel` | Dynamic |

---

## 4. Post-Build Deployment and Cleanup

1. **Root Binary Deployment**: All compiled executables (`baspp.exe`, `bpp.exe`, `bs.exe`, `iot.exe`, `bppc.exe`, `trans.exe`, `detok.exe`) and shared libraries (`basicpp.dll`, `SDL2.dll`) are copied directly to the repository root for immediate non-path execution.
2. **Synchronous Intermediate Cleanup**: Intermediate compiler cruft (`.obj`, `.o`, `.tlog`, `.idb`, `.pdb`, `*.dir`) is purged synchronously via `tools/clean_intermediates.ps1` on Windows or `tools/clean_intermediates.sh` on Linux, keeping repository size minimal.
