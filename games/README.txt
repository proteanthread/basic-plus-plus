BASIC++ Game Collection
=======================

Text-based arcade and puzzle games demonstrating
the BASIC++ game engine capabilities.

Requirements:
  - BASIC++ interpreter (basicpp.exe)
  - Terminal with ANSI escape support
  - 80x25 minimum terminal size

Running Games:
  basicpp games/invaders.bas
  basicpp games/pacman.bas
  basicpp games/missile.bas
  basicpp games/donkeykong.bas
  basicpp games/mario.bas
  basicpp games/defender.bas
  basicpp games/zelda.bas
  basicpp games/solitaire.bas
  basicpp games/pegsol.bas

Game Library:
  MODULE "GAME.LIB" - load game helper subroutines

Controls:
  Arrow keys  - Move player / cursor
  SPACE       - Fire / Jump / Select
  ESC         - Quit game
  (see individual games for extras)

Games
-----

INVADERS.BAS - Space Invaders
  5x11 alien grid, shields, multiple bullets,
  enemy bombs, levels, scoring.

PACMAN.BAS - Pac-Man
  20x21 maze, 4 ghosts, dot collection,
  power pellets, tunnel wrap, directional char.

MISSILE.BAS - Missile Command
  Crosshair defense, 6 cities, incoming missiles
  with trails, counter-missile explosions, ammo.

DONKEYKONG.BAS - Donkey Kong
  5 platforms, ladders, rolling barrels,
  jumping with gravity, princess rescue.

MARIO.BAS - Mario Brothers
  3-tier platforms, pipe-spawned enemies,
  bump-from-below mechanic, POW block.

DEFENDER.BAS - Defender
  200-column scrolling world, directional ship,
  laser fire, 3 enemy types, smart bombs, minimap.

ZELDA.BAS - Zelda Adventure
  Room-based overworld, procedural generation,
  sword combat, keys/doors, health hearts.

SOLITAIRE.BAS - Klondike Solitaire
  Full 52-card deck, 7-column tableau,
  stock/waste, foundations, cursor navigation.

PEGSOL.BAS - Peg Solitaire
  English cross board, jump-removal puzzle,
  move validation, win/lose detection.

Engine Features Used
--------------------
  SCREEN LOCK/UNLOCK  - Flicker-free double buffering
  INKEY$ (extended)   - 2-char string for arrow/F-keys
  TICKS               - Millisecond frame rate control
  LOCATE/COLOR        - Character-precise rendering
  SCREEN()            - Read-back collision detection
  CURSOR ON/OFF       - Hidden cursor during gameplay
  DIM arrays          - Game object storage
  SOUND/BEEP          - Sound effects
  CHR$()              - CP437 character sprites
