# Arcade Game Engines

BASIC++ includes several modular libraries for building action, arcade, and real-time game genres. These can be combined with each other to construct more complex game physics and AI.

## ARCADE_SHOOTER.LIB

Provides basic mechanisms for vertical or horizontal shooters (e.g., Space Invaders, Defender, Missile Command).

- **AS_INIT_SWARM(COUNT)**: Initializes a swarm of generic enemies.
- **AS_UPDATE_BULLETS()**: Moves all active projectiles on the screen.
- **AS_CHECK_COLLISIONS()**: Evaluates bounding box intersections between player bullets and enemy ships, triggering explosions.

## PLATFORMER.LIB

Provides 2D physics simulations for side-scrolling or single-screen platformers (e.g., Mario, Donkey Kong).

- **PF_APPLY_GRAVITY()**: Applies downward velocity to all active entities not colliding with a floor tile.
- **PF_CHECK_FLOOR(X, Y)**: Returns TRUE if the coordinate contains a solid block.
- **PF_JUMP()**: Imparts upward velocity to the player character if they are currently grounded.

## SWARM.LIB

Provides advanced flocking and swarming behaviors, typically used alongside ARCADE_SHOOTER.LIB.

- **SW_UPDATE_POSITIONS()**: Moves a group of entities in a synchronized sinusoidal or directional pattern.
- **SW_DIVE_BOMB(INDEX)**: Commands a specific entity to break formation and chase the player coordinates.

## MAZE.LIB

Provides maze generation and layout tools (e.g., Pac-Man).

- **MZ_GENERATE(WIDTH, HEIGHT)**: Creates a cellular automaton or recursive backtracking maze layout.
- **MZ_DRAW()**: Renders the generated maze with appropriate ASCII wall tiles.
- **MZ_CHECK_WALL(X, Y)**: Verifies if a given map coordinate is passable.
