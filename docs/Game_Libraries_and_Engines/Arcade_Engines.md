# Arcade Game Engines

BASIC++ includes a suite of specialized companion game libraries designed for action, arcade, real-time physics, and entity simulation. These modular libraries can be combined to build vertical shooters, side-scrolling platformers, flocking swarm AI, and maze navigation games.

## Available Engine Modules

```
+---------------------------------------------------------------+
|                      ARCADE GAME SUITE                        |
+-------------------+-------------------+-----------------------+
| ARCADE_SHOOTER    | PLATFORMER        | SWARM & MAZE          |
| - Projectile Pool | - Gravity Vector  | - Flocking Trajectory |
| - Swarm Tracking  | - Jump Impulses   | - Backtracker Mazes   |
| - AABB Collision  | - Solid Tile Maps | - Wall Passability    |
+-------------------+-------------------+-----------------------+
```

---

## 1. Fixed and Vertical Shooter Engine (`ARCADE_SHOOTER.LIB`)

The `ARCADE_SHOOTER.LIB` module provides high-speed entity pooling, projectile lifecycle tracking, and Axis-Aligned Bounding Box (AABB) collision detection tailored for classic space shooters (e.g. *Space Invaders*, *Galaxian*, *Defender*).

### Procedures & API

- **`AS_INIT_SWARM(Count%)`**  
  Initializes an array of `Count%` enemy ships across horizontal rows. Configures initial velocity, health, and screen boundary limits.
- **`AS_UPDATE_BULLETS()`**  
  Advances all active player and enemy bullet positions by their velocity vectors. Automatically recycles projectiles that exit visible screen bounds.
- **`AS_CHECK_COLLISIONS()`**  
  Performs 2D AABB bounding-box intersection tests between active bullets and enemy entities. Triggers explosion states and increments score counters on hit.

### Code Example: Shooter Main Loop
```basic
10 REM Space Shooter Loop
20 CALL AS_INIT_SWARM(20)
30 WHILE GameRunning%
40     K$ = INKEY$
50     IF K$ = " " THEN CALL AS_FIRE_BULLET(PlayerX, PlayerY)
60     IF K$ = "A" AND PlayerX > 1 THEN PlayerX = PlayerX - 1
70     IF K$ = "D" AND PlayerX < 78 THEN PlayerX = PlayerX + 1
80     CALL AS_UPDATE_BULLETS()
90     CALL AS_CHECK_COLLISIONS()
100    CALL AS_RENDER_SCENE(PlayerX, PlayerY)
110    _TIMER_DELAY 0.016 : REM ~60 FPS loop
120 WEND
```

---

## 2. Platformer & Physics Engine (`PLATFORMER.LIB`)

The `PLATFORMER.LIB` module provides 2D kinematic physics simulation, including downward gravitational acceleration, jump impulse velocity, and tilemap collision checking for platformer mechanics (e.g. *Mario*, *Donkey Kong*).

### Procedures & API

- **`PF_APPLY_GRAVITY()`**  
  Applies gravitational acceleration ($v_y = v_y + g$) to all active entities not grounded on a solid surface. Clamps maximum terminal fall velocity.
- **`PF_CHECK_FLOOR(X%, Y%)`**  
  Evaluates whether the coordinate `(X%, Y%)` in the active level tilemap contains a solid or passable platform tile. Returns `TRUE` (-1) or `FALSE` (0).
- **`PF_JUMP(Force#)`**  
  Imparts an immediate upward vertical velocity impulse if the player entity is currently grounded, preventing infinite mid-air jumps.

### Code Example: Platformer Movement
```basic
100 REM Update Player Kinematics
110 IF PF_CHECK_FLOOR(PX%, PY% + 1) THEN
120     IsGrounded% = 1
130     VelY = 0
140 ELSE
150     IsGrounded% = 0
160     CALL PF_APPLY_GRAVITY()
170 END IF
180 IF INKEY$ = "W" AND IsGrounded% THEN CALL PF_JUMP(4.5)
```

---

## 3. Swarm & Multi-Entity AI (`SWARM.LIB`)

The `SWARM.LIB` module implements synchronized formation movement, flocking algorithms, and individual entity combat AI (e.g. Galaga dive-bombing).

### Procedures & API

- **`SW_UPDATE_POSITIONS()`**  
  Updates the global position offsets for the entire active swarm using sinusoidal horizontal oscillation coupled with downward step shifts upon hitting screen margins.
- **`SW_DIVE_BOMB(EnemyIndex%)`**  
  Detaches entity `EnemyIndex%` from the main formation, switching its AI state to a quadratic Bezier or direct homing trajectory targeted at the player coordinates.

---

## 4. Procedural Maze Generator (`MAZE.LIB`)

The `MAZE.LIB` module implements randomized maze carving algorithms (recursive backtracking and cellular automata) suitable for top-down dungeon and maze games (e.g. *Pac-Man*).

### Procedures & API

- **`MZ_GENERATE(Width%, Height%)`**  
  Generates a fully connected 2D maze layout with guaranteed path traversability and single-tile boundary walls.
- **`MZ_DRAW(StartX%, StartY%)`**  
  Renders the generated maze onto the active virtual console (`VCon`) or graphical screen buffer using box-drawing characters or graphics tiles.
- **`MZ_CHECK_WALL(X%, Y%)`**  
  Returns `TRUE` (-1) if coordinate `(X%, Y%)` is an impassable wall, preventing entity clipping.

---

## Cross-References

- **`Game_Libraries_and_Engines/Overview.md`** — Architecture overview, global return registers, and save/load workflows.
- **`Game_Libraries_and_Engines/Boardgame.md`** — Grid-based boardgame engine.
- **`Game_Libraries_and_Engines/RPG_ProcGen.md`** — Procedural dungeon generation.
- **`Screen_And_Console.md`** — Screen buffer control and double-buffering.

---

## Proposed Expansion or Changes

In legacy BASIC++ v5.0.5, these arcade routines were distributed as discrete `.LIB` source modules.

### v6.5.2 Native Integration Plan:
1. **C17 Vectorized Collision**: Implement `collision_aabb_test()` in `engine/src/device/bgi/` to execute bounding-box collision detection natively in C17 rather than interpreted BASIC loops.
2. **Dedicated Sprite Engine**: Integrate `ARCADE_SHOOTER` with v6.5.2 virtual sprites (`PUTIMAGE`, `GETIMAGE`, `FREEIMAGE`) for hardware-accelerated SDL2/OpenGL rendering.
3. **Pre-compiled Bytecode Module**: Bundle `ARCADE_SHOOTER.BPC` into the standard library distribution.
