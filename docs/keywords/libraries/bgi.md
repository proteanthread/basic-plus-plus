# `bgi` BASIC++ Graphics Interface Subsystem (`libbgi` / `libadvanced`)

## 1. Architectural Purpose & Overview

The `bgi` subsystem (`engine/src/statements/bgi/` and `engine/src/graphics/`) implements the BGI Layered Resolution Engine, retro hardware modes, primitives rasterization, font engines, and split-screen text windows.

### Key Architectural Invariants:
- **Custom Initializers**: Supports `SET SCREEN` (default 256 colors, full screen), `SET GRAPHICS` (default 16 colors, 80x4 split screen), and `SET MODE` (42 retro hardware profiles).
- **Split-Screen Text Window**: 1x1, 2x1, 2x2 font scaling with dedicated text scrolling buffer.
- **Delay-Loaded Host Backend**: Desktop edition delay-loads `SDL2.dll` on demand when graphics modes are activated.

---

## 2. Technical API Signatures (C17)

```c
void bgi_init(void);
void bgi_set_mode(int mode_id);
void bgi_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void bgi_draw_circle(int cx, int cy, int radius, uint32_t color);
void bgi_fill_rect(int x1, int y1, int x2, int y2, uint32_t color);
```
