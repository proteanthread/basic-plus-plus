# `zelda` Top-Down Action RPG Room Engine Library

## 1. Library Overview & Usage

The `zelda` game library provides top-down multi-room flip-screen scrolling, tile collisions, sword/arrow combat hitboxes, inventory item selection, and dungeon door state persistence.

### Features:
- **Room Flip Transition**: Smooth screen-slide transitions when the player crosses screen boundaries.
- **Tile Collisions**: Solid trees, water barriers, pushable blocks, and locked doors.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 DIM OVERWORLD_MAP(16, 8)
30 PRINT "Top-down action RPG engine active (16x8 room overworld)."
```
