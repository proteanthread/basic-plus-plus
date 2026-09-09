# `invaders` Classic Fixed-Matrix Arcade Shooter Template

## 1. Library Overview & Usage

The `invaders` template implements the classic $5\times 11$ marching alien grid, destructible bunker shield bitmaps, marching sound cadence, and mystery flying saucers.

### Features:
- **Matrix Grid March**: Alien armada step animation with speed increasing as alien count decreases.
- **Destructible Shields**: Pixel-level collision and erosion of protective bunkers.

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 DIM INVADER_ROW(5), INVADER_COL(11)
30 PRINT "Space Invaders grid initialized."
```
