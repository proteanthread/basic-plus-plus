# Game Engine Master Guide

BASIC++ 5.0.1 introduced a modular architecture for the 11 built-in games (Adventure, Zelda, Pacman, Mario, Donkey Kong, Defender, Missile Command, Peg Solitaire, Solitaire, Starfield, and Space Invaders). 

This guide explains how the engines themselves operate.

## Architecture Overview

All engines are standard BASIC++ `.BAS` files found in the `games/` directory. They contain **zero** hardcoded rendering logic other than standard text-mode `LOCATE`, `COLOR`, and `PRINT` commands.

### Dependency Injection
Each engine relies on one or more external `.LIB` files for its mechanics. For instance:
- `mario.BAS` and `donkeykong.BAS` load `platformer.LIB`.
- `pacman.BAS` loads `maze.LIB`.

The engine simply uses `LOAD LIBRARY "lib\library_name.LIB"` at the top of the file to import the subroutines.

### Game Loops
All engines use a tight, non-blocking input loop using `INKEY$` combined with the `TICKS` timer to maintain a consistent frame rate. 

```basic
GFT = TICKS
... (Update logic)
GT2 = TICKS: GT1 = GT2 - GFT
IF GT1 < 50 THEN DELAY 50 - GT1
```

### Save States (`.SAV`)
All game engines support persistent progress saving. Pressing `S` during gameplay will `GOSUB 8000` (or similar) to serialize variables to a `<game>.sav` file via `OPEN ... FOR OUTPUT`. Pressing `L` will deserialize the variables.

## List of Engines

| Game | Library Dependency | Description |
|---|---|---|
| `adventure.BAS` | `rpg_procgen.LIB` | Node-based procedural adventure |
| `zelda.BAS` | `rpg_procgen.LIB` | Cellular automata grid procedural action-adventure |
| `pacman.BAS` | `maze.LIB` | Grid-based pathing and collision |
| `mario.BAS` | `platformer.LIB` | Gravity and jumping mechanics |
| `donkeykong.BAS` | `platformer.LIB` | Gravity and ladders |
| `defender.BAS` | `arcade_shooter.LIB` | Side-scrolling vector terrain |
| `missile.BAS` | `arcade_shooter.LIB` | Vector-based radii explosion mechanics |
| `pegsol.BAS` | `boardgame.LIB` | Grid cursor and validation mechanics |
| `solitaire.BAS` | `boardgame.LIB` | Card shuffling and drawing |
| `starfield.BAS` | `vfx.LIB` | 3D projection rendering |
| `invaders.BAS` | `swarm.LIB` | Grid-based multi-entity movement |
