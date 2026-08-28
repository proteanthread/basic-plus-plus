# `vfx` Visual Effects & Particle System Library (`libadvanced`)

## 1. Architectural Purpose & Overview

The `vfx` subsystem (`engine/src/graphics/vfx.c`) provides 2D/3D particle emitter physics, screen fades, palette rotation effects, and camera shake algorithms in BGI.

### Key Architectural Invariants:
- **Particle Emitters**: Fire, smoke, sparks, explosion bursts with gravity and drag.
- **Palette Transitions**: Smooth screen fade-to-black, whiteout flash, and palette cycle animations.

---

## 2. Technical API Signatures (C17)

```c
void vfx_spawn_particles(int x, int y, int count, uint32_t color, float speed);
void vfx_screen_fade(float factor);
void vfx_screen_shake(int intensity, int duration_frames);
```
