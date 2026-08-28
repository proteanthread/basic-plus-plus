# `starfield` 3D Perspective Warp Starfield Simulation

## 1. Library Overview & Usage

The `starfield` graphics library implements 3D point perspective projection ($X/Z$, $Y/Z$), speed acceleration, star streaking / motion blur lines, and color depth fading in BGI.

### Features:
- **Perspective Mathematics**: Screen coordinates projected via:
  $$X_{\text{screen}} = \text{Center}_X + \frac{X}{Z} \times \text{FOV}, \quad Y_{\text{screen}} = \text{Center}_Y + \frac{Y}{Z} \times \text{FOV}$$
- **Motion Streaks**: Renders vector tails between previous and current projected coordinates during warp drive.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 NUM_STARS = 200 : DIM SX(200), SY(200), SZ(200)
30 PRINT "3D Starfield warp engine active."
```
