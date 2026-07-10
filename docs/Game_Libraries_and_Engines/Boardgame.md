# Boardgame Engine (BOARDGAME.LIB)

The Boardgame engine provides robust tools for rendering grid-based logic such as board boards, cards, and puzzle elements.

## Initialization

- **BG_INIT_BOARD(W, H)**
  Initializes the background dimension boundaries for width W and height H.
  Allocates the internal arrays and coordinate maps.

## Rendering

- **BG_DRAW_BOARD()**
  Renders the currently established board onto the console using active visual styling.

- **BG_PRINT_CELL(X, Y, C$, COL)**
  Prints a character C$ with color COL at the specified grid coordinate X, Y.

## Cursors and Interaction

- **BG_MOVE_CURSOR(DX, DY, BRD_X, BRD_Y)**
  Moves the internal interaction cursor by DX and DY bounds-checked against the board edges.
  Updates BRD_X and BRD_Y globally.

## Subroutine Returns

Due to interpreter limitations on BYREF, any subroutine that fetches grid cell data or card strings will return its values in the global arrays:
- BG_RET_STR1: Contains the returned string data.
- BG_RET_COL(1): Contains the returned color attribute data.
