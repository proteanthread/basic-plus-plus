# `platformer` General 2D Platform Engine & Physics Library

## 1. Library Overview & Usage

The `platformer` engine library provides a comprehensive framework for 2D platformers: acceleration, friction, jump curves, moving platforms, and slope collisions in BGI.

### Features:
- **Kinematic Physics**: Sub-pixel precision positioning, variable jump damping, and terminal fall velocities.
- **Moving Platforms**: Passenger entity attachment and moving collision boxes.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 MAX_SPEED# = 3.5 : ACCEL# = 0.4 : FRICTION# = 0.85
30 PRINT "Platformer kinematics active."
```
