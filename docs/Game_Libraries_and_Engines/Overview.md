# Game Libraries and Engines Overview

BASIC++ provides a modular architecture for developing 2D text, character-mapped, and graphical games rapidly. By combining structured BASIC procedures with specialized reusable game libraries, developers can create grid-based strategy games, roguelikes, procedural dungeon crawlers, arcade fixed shooters, and platformers.

## Architectural Model

Game development in BASIC++ leverages the core engine's decoupled virtual devices (`VDev`, `VCon`), keyboard input event pumps, audio synthesizer (`BEEP`, `SOUND`, `PLAY`), and BGI graphics modes (`SET SCREEN`, `SET GRAPHICS`, `SCREEN`). 

High-level game mechanics are packaged into structured companion libraries (`.LIB` files) or modular units loaded into the interpreter:

```
+----------------------------------------------------------------+
|                        Game Application                        |
+-------------------------------+--------------------------------+
|   BOARDGAME.LIB / MAZE.LIB   |  ARCADE_SHOOTER.LIB / SWARM.LIB |
+-------------------------------+--------------------------------+
|                Procedural Generation (RPG_PROCGEN.LIB)         |
+----------------------------------------------------------------+
|           BASIC++ Virtual Machine & Runtime Subsystems         |
|     (VDev Console, VMem, String Pool, Dynamic Array Engine)    |
+----------------------------------------------------------------+
```

## Standard Companion Libraries

| Library | Target Genre / Domain | Primary Capabilities |
|---------|-----------------------|----------------------|
| `BOARDGAME.LIB` | Chess, Checkers, Puzzles, Solitaire | Grid initialization, 2D array coordinates, cursor navigation, cell rendering |
| `RPG_PROCGEN.LIB` | Roguelikes, RPGs, Dungeon Crawlers | Procedural room layout generation, cellular automaton caves, monster spawners, door states |
| `ARCADE_SHOOTER.LIB` | Space Invaders, Galaxian, Shmups | Enemy swarm tracking, projectile movement, bounding-box collision detection |
| `PLATFORMER.LIB` | Side-scrollers, Action Platformers | Gravity acceleration, floor/solid tile checking, jump impulses |
| `SWARM.LIB` | Real-time AI, Flocking Behaviors | Coordinated sinusoidal movement, dive-bomb targeting algorithms |
| `MAZE.LIB` | Mazes, Pac-Man style games | Recursive backtracker and cellular maze generation, impassable wall queries |

## Loading and Managing Libraries

In structured BASIC++ programs, library subroutines and data structures are included using standard file inclusion, module loading (`MODULE`), or program chaining:

```basic
10 REM Load Game Subsystems
20 LOAD LIBRARY "BOARDGAME"
30 LOAD LIBRARY "SWARM"
40 GOSUB 1000 : REM Initialize subsystems
```

To list loaded modules in an interactive session:
```basic
LIBRARY LIST
```

## State Management and Global Return Channels

Because historical Microsoft BASIC dialects and standard SUB subroutines pass arguments by value or simple references without arbitrary compound tuple returns, BASIC++ game libraries employ **Global Return Channels** for subroutines that generate multi-dimensional query results.

For example, `BOARDGAME.LIB` and `RPG_PROCGEN.LIB` define dedicated return registers:
- `BG_RET_STR1` — String result (e.g. selected piece ID, cell ASCII symbol).
- `BG_RET_COL(1)` — Color attribute register associated with the cell.
- `RPG_RET_STATUS` — Success or collision status code.

### Usage Pattern:
```basic
100 REM Query board coordinate at X=3, Y=4
110 CALL BG_GET_CELL(3, 4)
120 Tile$ = BG_RET_STR1
130 Attr% = BG_RET_COL(0)
140 PRINT "Tile at (3,4) is "; Tile$; " with color "; Attr%
```

## Saving and Restoring Game State

BASIC++ applications implement persistent game state, save slots, and configuration checkpoints using standard sequential and random-access file I/O or persistent VM state statements:

```basic
100 REM Save Game Checkpoint
110 OPEN "SLOT1.SAV" FOR OUTPUT AS #1
120 PRINT #1, PlayerX
130 PRINT #1, PlayerY
140 PRINT #1, Score%
150 PRINT #1, Level%
160 CLOSE #1
170 PRINT "Game successfully saved to SLOT1.SAV"
```

For advanced snapshot serialization, `_STATESAVE "checkpoint.sav"` can serialize the entire active VM state.

## Cross-References

- **`Arcade_Engines.md`** — Detailed API and algorithms for arcade shooters, platformers, swarms, and mazes.
- **`Boardgame.md`** — Grid setup, cursor handling, and visual cell rendering.
- **`RPG_ProcGen.md`** — Dungeon layout generation, entity spawners, and collision.
- **`Screen_And_Console.md`** — Console cursor control, color palettes, and text attributes.

---

## Proposed Expansion or Changes

In legacy BASIC++ v5.0.5, companion `.LIB` game engines were loaded into memory via an experimental `LOAD LIBRARY "name"` syntax and inspected with `LIBRARY LIST`.

### v6.5.2 Native Integration Plan:
1. **Module System Unification (`engine/src/modules/`)**: Unify game engine `.LIB` files with the native v6.5.2 `MODULE` / `UNLOAD` and companion library system (`mem_companion_get_all_lines()` in `engine/include/memory/memory.h`).
2. **Bytecode Game Runtime**: Provide pre-compiled bytecode headers for `BOARDGAME`, `RPG_PROCGEN`, and `ARCADE_SHOOTER` in `libstandard` / `libadvanced` to avoid parsing overhead during 60 FPS game loops.
3. **Hardware Acceleration**: Connect collision routines (`AS_CHECK_COLLISIONS`) to native C17 SIMD/bounding-box routines in `engine/src/device/bgi/`.
