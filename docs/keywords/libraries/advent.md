# `advent` Interactive Fiction & Text Adventure Engine Library

## 1. Library Overview & Usage

The `advent` library provides room navigation, item inventory management, verb-noun command parsing, and state serialization for text adventure games in BASIC++.

### Features:
- **Room Graph**: 2D/3D room connection matrices with compass directions (`N`, `S`, `E`, `W`, `U`, `D`).
- **Inventory Engine**: Item pickup, drop, inspection, and capacity constraints.

---

## 2. BASIC Example

```basic
10 DIM ROOM_DESC$(10), EXITS(10, 4)
20 ROOM_DESC$(1) = "You are standing at the end of a road before a small brick building."
30 PRINT ROOM_DESC$(1)
```
