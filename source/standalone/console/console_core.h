/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: console_core.h
 * Subsystem: Virtual Text Terminal Screen Buffer Manager
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Handles text rendering grids, cursor positioning, page scrolling, and character attributes.
 *
 * 2. WHAT TO EXPECT:
 *    Writes characters to cell arrays and updates viewports.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Dimension boundaries, scrolling speeds, character mapping templates.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Grid address translation algorithms.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If display glitches occur, check viewport bounds and clear buffers.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE VIRTUAL CONSOLE FRAMEBUFFER
 * File: console_core.h
 * ===================================================================== */

#ifndef STANDALONE_CONSOLE_CORE_H
#define STANDALONE_CONSOLE_CORE_H

#define CONSOLE_MAX_COLS 128
#define CONSOLE_MAX_ROWS 128

typedef struct {
    char chars[CONSOLE_MAX_ROWS][CONSOLE_MAX_COLS];
    unsigned char fg[CONSOLE_MAX_ROWS][CONSOLE_MAX_COLS];
    unsigned char bg[CONSOLE_MAX_ROWS][CONSOLE_MAX_COLS];
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    int scroll_start;
    int scroll_lines;
} ConsoleGrid;

void console_grid_init(ConsoleGrid *grid, int cols, int rows);
void console_grid_clear(ConsoleGrid *grid, unsigned char bg_color);
void console_grid_write_char(ConsoleGrid *grid, char c, unsigned char fg, unsigned char bg);
void console_grid_scroll(ConsoleGrid *grid, unsigned char fg, unsigned char bg);
void console_grid_move_cursor(ConsoleGrid *grid, int x, int y);

#endif /* STANDALONE_CONSOLE_CORE_H */
