# `rpg_procgen` Procedural Dungeon Generation & Roguelike Engine

## 1. Library Overview & Usage

The `rpg_procgen` library provides procedural room/corridor generation via Binary Space Partitioning (BSP) trees, cellular automata cave sculpting, and turn-based roguelike entity management.

### Features:
- **BSP Tree Dungeon Generator**: Recursively subdivides space into distinct rooms connected by hallways.
- **Field of View (FOV)**: Shadowcasting algorithm for fog-of-war and line-of-sight calculation.

---

## 2. BASIC Example

```basic
10 DIM DUNGEON(40, 25)
20 PRINT "Generating procedural BSP dungeon..."
```
