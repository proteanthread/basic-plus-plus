# `missile` Ballistic Intercept & Missile Defense Arcade Template

## 1. Library Overview & Usage

The `missile` template implements anti-ballistic missile trajectory calculations, expanding flak explosion radius physics, and city defense management in BGI.

### Features:
- **Trajectory Interpolation**: Calculates intercept vectors and speed for defensive counter-missiles.
- **Expanding Blast Waves**: Expanding and contracting circular blast areas that destroy incoming ICBMs.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 LINE (160, 190)-(100, 50), 14 : REM Intercept trajectory
30 CIRCLE (100, 50), 15, 12       : REM Flak burst
```
