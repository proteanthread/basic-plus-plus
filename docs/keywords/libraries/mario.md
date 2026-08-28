# `mario` Side-Scrolling Tile Platformer Template

## 1. Library Overview & Usage

The `mario` game template implements horizontal tilemap scrolling, variable-height jumping physics, question-block coin bumping, and enemy stomping mechanics in BGI.

### Features:
- **Tile Collision Grid**: Solid ground, breakable brick blocks, and prize question boxes.
- **Platformer Physics**: Variable jump velocity, gravity, running momentum, and friction.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 GRAVITY# = 0.5 : VEL_Y# = 0.0
30 PRINT "Platformer physics active."
```
