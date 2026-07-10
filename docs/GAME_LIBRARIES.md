# Game Mechanics Libraries Guide

This guide details the 7 core mechanics libraries located in `lib/` which power the BASIC++ game engines.

## 1. `swarm.LIB`
Handles grid-based multi-entity mechanics (e.g. Space Invaders).
- `CALL SWM_INIT(num_entities)`: Initializes the swarm arrays.
- `CALL SWM_UPDATE(dir_x, dir_y)`: Moves all active entities by the specified delta.
- `CALL SWM_CHECK_HIT(px, py)`: Checks if a given coordinate intersects with an entity.

## 2. `maze.LIB`
Handles pacman-style grid pathfinding and wall collision.
- `CALL MZ_INIT_BOARD(width, height)`: Sets up the grid array.
- `CALL MZ_SET_WALL(x, y)`: Marks a cell as impassable.
- `CALL MZ_MOVE_ENT(id, dx, dy)`: Moves an entity while respecting walls.

## 3. `platformer.LIB`
Physics engine for side-scrolling gravity and jumping.
- `CALL PL_INIT(gravity, terminal_vel)`: Sets physics constants.
- `CALL PL_APPLY_GRAVITY(y, vy)`: Updates Y-velocity and position.
- `CALL PL_JUMP(vy, force)`: Applies upward force if grounded.

## 4. `arcade_shooter.LIB`
Handles free-movement vector logic and explosive radii.
- `CALL AS_INIT_VECTORS(max_objects)`: Initializes coordinate tracking.
- `CALL AS_MOVE(id, vx, vy)`: Updates an object's position based on its velocity vector.
- `CALL AS_RADIUS_CHECK(x, y, r)`: Returns all object IDs within radius `r`.

## 5. `rpg_procgen.LIB`
Procedural generation for maps and dungeons.
- `CALL RPG_INIT_MAZE(num_rooms)`: Sets up the graph.
- `CALL RPG_GEN_MAZE()`: Uses randomized walking to connect rooms (North/South/East/West) ensuring pathability. Used in Adventure.
- `CALL RPG_GEN_GRID(width, height, fill_pct)`: Uses cellular automata to generate smooth cavern-like grid maps. Used in Zelda.

## 6. `boardgame.LIB`
Logic for cards, decks, and grid cursors.
- `CALL BG_INIT_DECK(size)`: Initializes an array 1..size.
- `CALL BG_SHUFFLE_DECK()`: Randomizes the array using Fisher-Yates.
- `CALL BG_MOVE_CURSOR(key, min_x, max_x, min_y, max_y, cx, cy)`: Translates an arrow key code into bounds-checked coordinate movement.

## 7. `vfx.LIB`
3D projection visual effects.
- `CALL VFX_INIT_STARFIELD(max_stars, depth)`: Initializes 3D coordinate arrays.
- `CALL VFX_UPDATE_STARS(speed, center_x, center_y, width, height)`: Moves stars closer on the Z-axis, projects them to 2D (X/Y), and handles rendering logic for perspective.

## Mixing and Matching
Because these libraries operate purely on arrays and variables (and do not tightly bind to the render loop), they can be imported alongside each other. For example, `rpg_procgen.LIB` can be used to generate a map, and `arcade_shooter.LIB` can be used to handle combat within that map!
