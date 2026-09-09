<!--
Title:        BGI Graphics Architecture and SDL2 Decomposition
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BGI Graphics Architecture and SDL2 Decomposition

## 1. Architectural Overview

The BASIC++ graphics and multimedia architecture decouples display presentation, windowing, and audio output from host operating system APIs and third-party libraries. All 2D drawing, vector commands, palette management, and audio synthesis operate in pure freestanding ISO C17 memory under `libhardware`, while SDL2 video and audio presentation are isolated within `libhal` behind abstract virtual table interfaces.

Under this architecture:
1. **Hardware Abstraction Layer (HAL) Encapsulation**: SDL2 video presentation, event pumping, and audio streaming are isolated within `libhal` (`hal_sdl2.c` and `hal_sdl2.h`) behind freestanding `HalVideo` and `HalAudio` interfaces.
2. **Freestanding BGI Master 2D Engine**: All raster operations, line clipping, shape rasterization, scanline flood filling, palette mapping, split-screen text window rendering, and vector macro language commands (`DRAW`) reside in freestanding C17 memory within `libhardware` (`bgi.h`, `bgi_core.c`, `bgi_raster.c`).
3. **Zero Direct SDL2 Dependencies in Engine Core**: Subsystems in `engine/src/device/` and `engine/src/statements/graphics/` never include `<SDL.h>` or call raw SDL2 functions. All drawing operates against an in-memory `BGI_Context` master canvas and routes display updates through `BGI_present()` and the HAL.
4. **Headless Zero-Window Operation**: On non-graphical targets (`bpp`, `bs`, `iot`), graphics statements execute safely into virtual memory without attempting to load `SDL2.dll` or initialize host display servers.

---

## 2. Structural Architecture & Data Flow

```
+-------------------------------------------------------------------------+
|                  BASIC++ AST Evaluator & Statement Dispatch             |
|   (SCREEN, COLOR, CLS, PSET, PRESET, LINE, CIRCLE, PAINT, DRAW, PLAY)    |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                    libengine Virtual Graphics Device                    |
|       (gfx.c, gfx_palette.c, gfx_tui.c, gfx_audio.c, primitives/)       |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                   libhardware Master BGI 2D Engine                      |
|       (BGI_Context, BGI_draw, BGI_draw_split_text, BGI_present)         |
+------------------------------------+------------------------------------+
                                     |
                                     v
+------------------------------------+------------------------------------+
|                     libhal Hardware Abstraction Layer                   |
|          (HalVideo, HalAudio, HalInput, HalMemory, HalTime)             |
+------------------------------------+------------------------------------+
                                     |
                  +------------------+------------------+
                  |                                     |
                  v                                     v
     +--------------------------+         +--------------------------+
     |  Hosted SDL2 HAL Driver  |         | Freestanding Bare-Metal  |
     | (hal_sdl2.c / delay-load)|         |   (Static Arena / VGA)   |
     +--------------------------+         +--------------------------+
```

---

## 3. Subsystem Breakdown & Implementation

### 3.1 HAL SDL2 Video & Audio Drivers (`hal_sdl2.c`, `hal_sdl2.h`)
- **`HalVideo` Interface**:
  - `init`: Creates hardware/software window and streaming texture. Delay-loaded on Windows to eliminate runtime dependencies when graphics are unused.
  - `shutdown`: Cleans up textures, renderers, and window handles.
  - `present_framebuffer`: Blits ARGB8888 software canvas to host texture and presents renderer.
  - `poll_events`: Processes host keyboard, mouse, and window quit events into VM event queues.
- **`HalAudio` Interface**:
  - `init`: Configures 44.1 kHz audio device specifications.
  - `tone`: Synthesizes pure square, triangle, and sine tones with frequency and duration metrics.

### 3.2 Freestanding BGI Master 2D Engine (`libhardware`)
- **`BGI_Context` Canvas**: Maintains pitch, width, height, active color, clipping rectangles, and a 32-bit ARGB software framebuffer.
- **Display Modes**: Supports standard retro PC modes (CGA 320x200 4-color, EGA 640x350 16-color, VGA Mode 13h 320x200 256-color, and SVGA packed-pixel modes).
- **Macro Drawing Engine (`DRAW`)**: Fully parses classic Microsoft `DRAW` command strings (`U`, `D`, `L`, `R`, `E`, `F`, `G`, `H`, `M`, `A`, `TA`, `C`, `B`, `N`) directly in freestanding C17 memory.

---

## 4. Verification & Quality Gates

1. **Zero Direct SDL2 Inclusions in Core Engine**: Audits across `engine/src/device/` and `engine/src/statements/graphics/` confirm zero `#include <SDL.h>`.
2. **Dedicated Unit Tests**:
   - `tests/hardware_freestanding_test.c`: Validates BGI mode registration, software rasterizer, clipping, and floodfill with zero windowing dependencies.
3. **Master Regression Coverage**:
   - `tests/qb_vbdos_master.bas`: 10/10 Packages PASSED (100%).
   - `tests/vintage_ecosystems_master.bas`: 14/14 Packages PASSED (100%).
