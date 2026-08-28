# `arcade_shooter` 2D Top-Down / Scrolling Arcade Shooter Template

## 1. Library Overview & Usage

The `arcade_shooter` template implements 2D sprite bullet pools, enemy wave spawners, collision detection grids, and parallax starfields in BGI.

### Features:
- **Bullet Pooling**: Pre-allocated projectile arrays with lifetime bounds.
- **AABB Collision Grid**: Fast bounding box collision checking between player lasers and enemy ships.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 COLOR 14 : PSET (160, 180) : REM Player ship
30 COLOR 12 : CIRCLE (160, 40), 8 : REM Enemy
```
