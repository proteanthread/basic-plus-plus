# Game Libraries and Engines Overview

BASIC++ includes several powerful game libraries and engines to help you build character-based games quickly. These libraries offer reusable subroutines for rendering grids, procedural map generation, entity management, and arcade physics.

## Available Libraries

- **BOARDGAME.LIB**: Grid-based game mechanics (chess, checkers, solitaire, pegsol, etc.). Handles 2D arrays, cursors, and rendering.
- **RPG_PROCGEN.LIB**: Procedural generation for Roguelikes and RPGs (Zelda, Adventure). Handles map layouts, monster spawning, and room transitions.
- **ARCADE_SHOOTER.LIB**: Mechanics for fixed shooters (Space Invaders). Handles sprite swarms, bullets, and collision.
- **PLATFORMER.LIB**: Physics and collision for platformers (Mario, Donkey Kong). Handles gravity, jumping, and level loading.
- **SWARM.LIB**: Advanced multi-entity AI and swarming behaviors.
- **MAZE.LIB**: Maze generation and pathfinding (Pac-Man).

## Loading Libraries

Use the LOAD LIBRARY command to import subroutines at runtime:
  LOAD LIBRARY "BOARDGAME"
  LOAD LIBRARY "RPG_PROCGEN"

You can verify successfully loaded libraries by typing LIBRARY LIST in immediate mode.

## Mixing and Matching

BASIC++ encourages "mixing and matching" libraries to create unique genres!
For example, you can build a grid-based RPG shooter by loading both BOARDGAME and SWARM! 
  LOAD LIBRARY "BOARDGAME"
  LOAD LIBRARY "SWARM"

Because the subroutines are prefixed uniquely (e.g., BG_ for Boardgame, RPG_ for RPG, SW_ for Swarm), they will not collide.

## State Management & Returns

Because BYREF is generally not supported for passing complex values safely back to the caller in BASIC++, all game libraries use Global Return Arrays for passing values out of a subroutine.

For example, BOARDGAME.LIB utilizes:
  DIM BG_RET_STR1
  DIM BG_RET_COL(1)

When you call a subroutine that needs to return multiple parameters, it will populate BG_RET_STR1 and BG_RET_COL(1). You must read these values immediately after the CALL returns.

## Saving and Loading Progress

To implement game saves, use native file I/O OPEN ... FOR OUTPUT to write your engine state to a .SAV file, and OPEN ... FOR INPUT to restore it. 
  OPEN "mygame.sav" FOR OUTPUT AS #1
  PRINT #1, PLAYER_X
  PRINT #1, PLAYER_Y
  PRINT #1, SCORE
  CLOSE #1
