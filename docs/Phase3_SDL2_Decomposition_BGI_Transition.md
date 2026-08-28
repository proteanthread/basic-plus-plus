# Phase 3: SDL2 Subsystem Decomposition & BGI Graphics Transition

## 1. Overview & Architectural Goals

Phase 3 transitions BASIC++ graphics, sound, and windowing architecture from legacy direct SDL2/libc dependencies into the unified **Freestanding 11-Modular Library Framework**:
1. **Hardware Abstraction Layer (HAL) Encapsulation**: SDL2 video presentation, event pumping, and audio output are completely isolated within `libhal` (`hal_sdl2.c` and `hal_sdl2.h`) behind freestanding `HalVideo` and `HalAudio` virtual table interfaces.
2. **Freestanding BGI Master 2D Engine**: All raster operations, line clipping, shape rasterization, scanline flood filling, palette mapping, split-screen text window rendering, and vector macro language commands (`DRAW`) reside in freestanding C17 memory within `libhardware` (`bgi.h`, `bgi_core.c`, `bgi_raster.c`).
3. **Zero Direct SDL2 Dependencies in Engine Core**: Subsystems in `engine/src/device/` (`gfx.c`, `gfx_palette.c`, `gfx_tui.c`, `gfx_audio.c`, `primitives/`) and `engine/src/statements/graphics/` no longer include `<SDL.h>` or call raw SDL2 functions. All drawing operates against the in-memory `BGI_Context` master canvas and routes display updates through `BGI_present()` and the HAL.
4. **Freestanding C17 Memory Safety**: Eliminated raw libc allocation and string functions in device code in favor of `runtime_malloc`, `runtime_free`, `runtime_memcpy`, and `runtime_snprintf`.

---

## 2. Structural Architecture & Component Map

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

## 3. Subsystem Breakdown & Key Implementations

### 3.1. HAL SDL2 Video & Audio Drivers (`hal_sdl2.c`, `hal_sdl2.h`)
- Implements `HalVideo` table:
  - `init`: Creates hardware/software window and streaming texture.
  - `shutdown`: Cleans up textures, renderers, and window handles.
  - `present_framebuffer`: Blits ARGB8888 software canvas to host texture and presents renderer.
  - `poll_events`: Processes host keyboard, mouse, and window quit events.
- Implements `HalAudio` table:
  - `init`: Configures 44.1 kHz audio device specifications.
  - `tone`: Synthesizes pure sine tones with frequency and duration metrics.
  - `beep`: Triggers standard 880 Hz audible alerts.
  - `stop`: Halts active tone synthesis.

### 3.2. BGI Vector DRAW Macro Language (`BGI_draw`)
- Fully compliant with GW-BASIC, BASICA, and QBASIC `DRAW` macro specifications:
  - Coordinate movement: `M x,y` (absolute and relative `+`/`-`).
  - Directional strokes: `U n` (Up), `D n` (Down), `L n` (Left), `R n` (Right).
  - Diagonal vectors: `E n` (Up-Right), `F n` (Down-Right), `G n` (Down-Left), `H n` (Up-Left).
  - Prefix modifiers: `B` (Blind move without plotting), `N` (No-update origin return).
  - Color selection: `C n` (Sets active palette color).

### 3.3. Retro Graphics Statements & Split-Screen Text
- **Heritage Screen Modes**: Support for CGA (`SCREEN 1`, `SCREEN 2`), Tandy (`SCREEN 3`), EGA (`SCREEN 9`, `SCREEN 10`), and VGA (`SCREEN 12`, `SCREEN 13`).
- **Retro Dialect Statements**: Complete implementations for Apple II (`GR`, `HGR`, `HGR2`, `HCOLOR`, `PLOT`, `HLIN`, `VLIN`, `HPLOT`), Atari (`GRAPHICS`, `DRAWTO`), and Sinclair (`BORDER`, `INK`, `PAPER`).
- **Split-Screen Text Window**: Rasterizes hardware text onto lower graphics rows with 1x1, 2x1, and 2x2 font scaling factors.

---

## 4. Verification & Validation Summary

| Test Suite | Location | Tests | Status |
| :--- | :--- | :---: | :---: |
| **libboot Freestanding Suite** | `boot_freestanding_test.exe` | 3 / 3 | **100% PASSED** |
| **libcore Runtime & HAL Suite** | `runtime_freestanding_test.exe` | 9 / 9 | **100% PASSED** |
| **libkernel Freestanding Suite** | `kernel_freestanding_test.exe` | 6 / 6 | **100% PASSED** |
| **libhardware Master BGI Suite** | `hardware_freestanding_test.exe` | 5 / 5 | **100% PASSED** |
| **libserver Freestanding Suite** | `server_freestanding_test.exe` | 4 / 4 | **100% PASSED** |
| **libscript Freestanding Suite** | `script_freestanding_test.exe` | 4 / 4 | **100% PASSED** |
| **libcore & libflex Suite** | `core_flex_freestanding_test.exe` | 5 / 5 | **100% PASSED** |
| **libstandard TUI Suite** | `standard_tui_freestanding_test.exe` | 32 / 32 | **100% PASSED** |
| **QuickBASIC & VB/DOS Master** | `qb_vbdos_master.bas` | 10 / 10 | **100% PASSED** |
| **Vintage Ecosystems Master** | `vintage_ecosystems_master.bas` | 14 / 14 | **100% PASSED** |
| **Vintage Deep Edge-Cases** | `vintage_deep_fuzz_stress.bas` | 8 / 8 | **100% PASSED** |
| **Built-in Diagnostics** | `SELFTEST` | 5 / 5 | **100% PASSED** |
