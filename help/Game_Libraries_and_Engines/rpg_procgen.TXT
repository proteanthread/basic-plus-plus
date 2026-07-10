# RPG Procedural Generation Engine (RPG_PROCGEN.LIB)

The RPG Procedural Generation Engine provides tools for generating dungeon maps, populating monsters, handling player movement, and processing room transitions.

## World Generation

- **RPG_GEN_MAP(LEVEL)**
  Generates a random procedural map Layout for the specified LEVEL (difficulty or depth).
  Automatically links adjacent rooms and places locked doors.
  It stores the map internally in the RPG_MAP array.

- **RPG_GEN_DUNGEON(SEED)**
  Seeds the random number generator and recursively builds a dungeon layout using the random seed.

## Entities and Interaction

- **RPG_SPAWN_MONSTERS(ROOM_X, ROOM_Y)**
  Populates the given room with random entities or enemies based on room difficulty.

- **RPG_DRAW_ROOM(ROOM_X, ROOM_Y)**
  Draws the physical walls, doors, floors, and entities of the specified room to the screen.

- **RPG_MOVE_ENTITIES()**
  Iterates over the monster list and applies basic chase/wander AI for the monsters in the current room.
  It also checks for collision with the player character.

## Game States

When interacting with doors, use RPG_CHECK_DOOR(DX, DY) to see if a door is locked. State returns are handled through standard global variables or inline checking.
