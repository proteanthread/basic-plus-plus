# BASIC++ on FreeDOS — Compilation & Memory Reference

**Version 4.2.3**

This document serves as the single authoritative guide for compiling, configuring, and optimizing BASIC++ for the FreeDOS platform.

---

## Table of Contents
1. [Why a Slim/Lite Build?](#1-why-a-slimlite-build)
2. [Building for FreeDOS](#2-building-for-freedos)
3. [Memory Budgets & Gating](#3-memory-budgets--gating)
4. [Dialect Selection & Suitability](#4-dialect-selection--suitability)
5. [Footprint Optimization Checklist](#5-footprint-optimization-checklist)
6. [How to Customize Dialects & Modules](#6-how-to-customize-dialects--modules)
7. [Differences from Windows/Linux](#7-differences-from-windowslinux)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Why a Slim/Lite Build?

FreeDOS runs in 16-bit real mode, which imposes a strict **640 KB conventional memory limit** on all executables. The standard build profile of BASIC++ includes 16 dialects and multiple optional modules (USB, FujiNet, UPnP, etc.), which requires generous allocations (~90 MB) and will instantly crash on real-mode DOS.

To run on FreeDOS, we compile:
1. **BASIC++ Standard Slim** (`BASIC_FREEDOS`): Excludes the 14 optional dialects and USB/FujiNet/UPnP modules to fit within conventional memory.
2. **BASIC++ Lite** (`BASIC_LITE_BUILD`): An ultra-minimal, integer-only version of the interpreter (`blite.exe`) with a strict **256 KB memory safety floor**.

---

## 2. Building for FreeDOS

### Prerequisites
*   OpenWatcom C/C++ compiler (`wcc` for 16-bit, `wcc386` for 32-bit).
*   `wmake` or GNU `make`.

### Cross-Compilation Targets (Makefile)

Compile from your development shell using the following targets:

```bash
# 16-bit Real Mode Slim Build (default, compatible with 8086+)
make watcom

# 32-bit DOS/4GW Protected Mode Extender Build (requires 386+)
make watcom386

# 16-bit Real Mode BASIC++ Lite Build (gated ultra-slim)
make watcom LITE=1
```

All targets generate the binary `baspp.exe` (or `blite.exe` for Lite) in the project root directory.

### Native FreeDOS Build Scripts
If compiling directly on a FreeDOS machine, use the native scripts:
*   `build-fd.bat` (16-bit default)
*   `build-fd 32` (32-bit DOS/4GW)
*   `build-fd clean` (clean build artifacts)

Direct `wmake` usage on FreeDOS:
```cmd
cd source
wmake -f makefd
wmake -f makefd BITS=32
wmake -f makefd clean
```

---

## 3. Memory Budgets & Gating

> [!IMPORTANT]
> **FreeDOS Gating Rule**: Under FreeDOS (`BASIC_FREEDOS`), the highly constrained low-memory pools (32 KB program buffer, 16 KB string pool, etc.) are **only enabled when compiling the Lite build** (`BASIC_LITE_BUILD` via `make watcom LITE=1`). 
> 
> Standard 16-bit (`watcom`) and 32-bit (`watcom386`) builds without `LITE=1` use the modern default pool sizes (8 MB program, 16 MB string pool) and require conventional memory tuning or a DOS memory extender.

### Memory Pools for FreeDOS Lite (`BASIC_FREEDOS` + `BASIC_LITE_BUILD`)

When both gates are active, the memory budget fits comfortably inside conventional RAM:

| Pool | Size | Purpose |
|------|------|---------|
| Program memory | 32 KB | Stored program text |
| Variable memory | 16 KB | Runtime variable storage |
| Scratch memory | 16 KB | Tokenizer/parser workspace |
| String pool | 16 KB | String values during RUN |
| Array pool | ~32 KB | DIM array elements (2048 x 16B) |
| **Approximate Total** | **~128 KB** | **Safely runs within 256 KB RAM** |

### Capacity Limits (FreeDOS Lite)
*   Maximum program lines: **1,024**
*   Maximum stack depth: **64**
*   Maximum named variables: **128**
*   Maximum DATA items: **1,024**
*   Maximum DIM arrays: **32**
*   Maximum user-defined functions: **32**
*   Maximum breakpoints: **16**
*   Graphics virtual resolution: **80 x 25**

---

## 4. Dialect Selection & Suitability

### Dialect Suitability in FreeDOS Lite

Selecting the correct dialect profile is key to maximizing program capacity under memory constraints:

*   **Palo Alto Tiny BASIC (`PATB`)** [Highly Suited]: Extremely lightweight. Instantly boots, has minimal startup allocations, and runs comfortably on any real-mode DOS system.
*   **TRS-80 Level I / II (`TRS80`)** [Highly Suited]: Compact keyword processing and simple syntax rules.
*   **GW-BASIC (`GWBS`)** [Moderate]: The standard dialect for FreeDOS application scripting. Fits comfortably alongside standard library modules.
*   **ECMA-116 (`E116`) / QBasic (`QBAS`)** [Unsuited]: Not recommended for Lite. They rely heavily on float matrices and structured logic, which are stripped from the Lite compiler gates.

### Default Dialects on FreeDOS Slim
At startup, the 16-bit/32-bit Slim FreeDOS builds default to **GW-BASIC** (`GWBS`) and also include **ECMA-116** (`E116`).

Switch dialects at runtime:
```basic
DIALECT "E116"   ' Switch to ECMA-116
DIALECT "GWBS"   ' Switch to GW-BASIC
```

---

## 5. Footprint Optimization Checklist

To ensure the smallest memory allocation on DOS:

1.  **OpenWatcom Size Flags**:
    Compile with `-os` (optimize for size) and `-bt=dos` (target DOS).
2.  **Define Static Bounds (`config.h`)**:
    Tighten pools in `config.h` under the `BASIC_FREEDOS` + `BASIC_LITE_BUILD` gate if fitting on extremely memory-constrained systems (e.g. 256 KB).
3.  **Dialect Pruning**:
    Only compile Palo Alto Tiny BASIC or GW-BASIC. Exclude other dialect files to let the linker strip unused dialect routines.
4.  **Watcom Linker Optimization Directives**:
    Include `option eliminate` in the OpenWatcom Linker (`wlink`) call. This enables linker dead-code elimination:
    ```lnk
    option quiet
    option eliminate
    ```

---

## 6. How to Customize Dialects & Modules

If compiling a custom build (especially for 32-bit `watcom386` with extended memory), you can restore dialects or modules:

### Adding a Dialect (e.g. QBasic)

1.  **dialect.h**: Remove the `#ifndef BASIC_FREEDOS` guard around the declaration:
    ```c
    ```
    ```c
    ```
3.  **Makefile**: Add `dialect_qbasic.obj` to `WATCOM_OBJS` and add the compile directive:
    ```makefile
    ```

### Restoring Optional Modules
Follow the same procedure:
1.  Remove the `#ifndef BASIC_FREEDOS` guard around the module registration in `source/core/boot.c`.
2.  Add the module `.obj` file to `WATCOM_OBJS` in the Makefile.
3.  Rebuild with `make watcom` / `make watcom386`.

---

## 7. Differences from Windows/Linux

| Feature | FreeDOS Lite | FreeDOS Slim | Windows/Linux |
|---|---|---|---|
| **Dialects** | 2 (`PATB`, `TRS80`) | 2 (`GWBS`, `E116`) | 16 (all) |
| **Optional Modules** | 0 | 0 | 3 (`USB`, `FujiNet`, `UPnP`) |
| **Program Memory** | 32 KB | 8 MB (default) | 8 MB |
| **String Pool** | 16 KB | 16 MB (default) | 16 MB |
| **Max Program Lines** | 1,024 | 65,536 | 65,536 |
| **Max Stack Depth** | 64 | 1,024 | 1,024 |

---

## 8. Troubleshooting

### "SORRY. Cannot allocate memory."
The interpreter cannot allocate enough conventional memory at startup.
*   If running the Standard Slim build, switch to the **Lite build** (`blite.exe`).
*   Free up conventional memory on FreeDOS by unloading unused drivers or TSRs.
*   Tighten pool sizes in `config.h` and rebuild.

### "Unknown dialect"
You attempted to load a dialect not compiled into the DOS binary. Check your active build configuration.
