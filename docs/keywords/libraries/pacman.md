# `pacman` Tile Maze & Ghost AI State Machine Arcade Template

## 1. Library Overview & Usage

The `pacman` template implements tile-grid maze navigation, screen wraparound side tunnels, pellet dot consumption, and the classic four-state ghost AI personality system (Chase, Scatter, Frightened, Eaten).

### Features:
- **Tile Alignment**: Buffer turning queue for smooth tile cornering.
- **Ghost AI Personalities**: Blinky (direct target), Pinky (intercept ahead), Inky (flanking vector), Clyde (proximity retreat).

---

## 2. BASIC Example

```basic
10 SET SCREEN 320, 200, 8
20 DIM GHOST_STATE(4) : REM 1=CHASE, 2=SCATTER, 3=FRIGHTENED, 4=EATEN
30 PRINT "Pacman maze & ghost AI state machine active."
```
