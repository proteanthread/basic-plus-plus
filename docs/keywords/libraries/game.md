# `game` Unified 2D Game Runtime Core Library

## 1. Library Overview & Usage

The `game` core library coordinates fixed-timestep game loops, delta timing, state machine scene managers, and audio-visual synchronization in BASIC++.

### Features:
- **Fixed Timestep**: Predictable physics updates independent of rendering frame rates.
- **Scene Manager**: Title screen $\to$ gameplay $\to$ pause $\to$ game over transitions.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 TARGET_FPS = 60
30 DT# = 1.0 / TARGET_FPS
40 PRINT "Game loop running at 60 FPS (dt = "; DT#; " s)"
```
