# BASIC++ v6.5.2 Subsystem Porting

## 1. OVERVIEW

When porting individual subsystems to new environments, each subsystem can be isolated from the full BASIC++ engine. The 12-library modular architecture allows subsystems to be extracted, adapted, and linked independently.

## 2. SUBSYSTEM ISOLATION

Each library in the chain has explicit dependencies:

- **libboot** → No dependencies (freestanding C17).
- **libplatform** → OS headers only (no engine dependencies).
- **libkernel** → libboot + libplatform.
- **libengine** → libkernel.
- **libhardware** → libkernel (BIOS, vmem are self-contained with libkernel API).
- **libserver** → libkernel + libplatform (network, filesystem).
- **libscript** → libengine (file I/O for batch runner).
- **libcore** → libengine (REPL, formatting).
- **libflex** → libengine (metaprogramming).
- **libstandard** → libcore + libflex (TUI, editor).
- **libadvanced** → libstandard (SDL2 graphics, multimedia).
- **libext** → Any preceding library.

## 3. EXTRACTING THE BIOS SUBSYSTEM

The BIOS subsystem (libbios, engine/src/bios/) can be extracted for use in other projects:

1. Copy engine/src/bios/ and engine/include/bios/.
2. The BIOS compiles as freestanding C17 using only `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`.
3. Implement the HAL dispatch functions for your target environment.
4. Link with your project.

The BIOS subsystem provides PC BIOS emulation without requiring the BASIC++ VM.

## 4. EXTRACTING THE BGI SUBSYSTEM

The BGI rasterizer (libbgi, engine/src/device/bgi/) can be extracted for standalone graphics:

1. Copy engine/src/device/bgi/ and engine/include/device/bgi/.
2. The BGI requires the font library (libfont) for text rendering.
3. Provide a framebuffer output function (SDL surface, raw memory, or file output).
4. Call BGI functions directly from C code.

## 5. EXTRACTING THE LEXER

The lexer (engine/src/lexer/lexer.c) can be extracted for use in tools that need to parse BASIC++ source:

1. Copy engine/src/lexer/ and engine/include/lexer/.
2. Copy engine/include/types/ for token and keyword definitions.
3. The lexer depends only on standard C17 library functions.

## 6. PORTING THE PLATFORM LAYER TO RTOS

For real-time operating systems (FreeRTOS, Zephyr, NuttX):

**plat_console**: Map to UART driver. Use interrupt-driven character reception for INKEY$.

**plat_fs**: Map to the RTOS filesystem (FATFS for SD cards, SPIFFS/LittleFS for flash).

**plat_time**: Map to the RTOS tick counter or hardware timer.

**plat_thread**: Map to RTOS mutex and semaphore primitives. Even single-threaded usage needs mutex stubs because some subsystems use mutexes for safety.

## 7. PORTING TO WEB (WASM)

BASIC++ can be compiled to WebAssembly using Emscripten:

**plat_console**: Map to JavaScript console API or a virtual terminal widget.

**plat_fs**: Map to Emscripten's virtual filesystem (MEMFS or IDBFS).

**plat_time**: Map to JavaScript Date.now() and setTimeout.

**plat_net**: Map to WebSocket API (XMLHttpRequest is synchronous and not suitable).

The WASM build excludes SDL2 graphics, dynamic library loading, and thread support.

## 8. SUBSYSTEM COMPILATION FLAGS

Each subsystem can be individually enabled or disabled:

```cmake
set(SUPPORT_GRAPHICS OFF)    # Exclude BGI
set(SUPPORT_NET OFF)          # Exclude networking
set(SUPPORT_VMEM OFF)         # Exclude segmented memory
set(SUPPORT_EDITOR OFF)       # Exclude TUI editor
set(SUPPORT_BIOS OFF)         # Exclude BIOS emulation
set(SUPPORT_SOUND OFF)        # Exclude audio
set(SUPPORT_MODULE OFF)       # Exclude module system
```

Disabled subsystems are compiled out entirely — they add zero code size and zero RAM usage.
