/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: console_core.c
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
 * File: console_core.c
 * ===================================================================== */

#include "console_core.h"
#include <string.h>

void console_grid_init(ConsoleGrid *grid, int cols, int rows)
{
    if (grid == NULL) return;
    if (cols > CONSOLE_MAX_COLS) cols = CONSOLE_MAX_COLS;
    if (rows > CONSOLE_MAX_ROWS) rows = CONSOLE_MAX_ROWS;
    
    grid->cols = cols;
    grid->rows = rows;
    grid->cursor_x = 0;
    grid->cursor_y = 0;
    grid->scroll_start = 0;
    grid->scroll_lines = rows;
    
    console_grid_clear(grid, 0);
}

void console_grid_clear(ConsoleGrid *grid, unsigned char bg_color)
{
    if (grid == NULL) return;
    for (int r = 0; r < grid->rows; r++) {
        memset(grid->chars[r], ' ', (size_t)grid->cols);
        memset(grid->fg[r], 7, (size_t)grid->cols); /* default grey */
        memset(grid->bg[r], bg_color, (size_t)grid->cols);
    }
    grid->cursor_x = 0;
    grid->cursor_y = 0;
}

void console_grid_scroll(ConsoleGrid *grid, unsigned char fg, unsigned char bg)
{
    if (grid == NULL) return;
    int start = grid->scroll_start;
    int num = grid->scroll_lines;
    if (start < 0) start = 0;
    if (num <= 0) num = grid->rows - start;
    if (start + num > grid->rows) num = grid->rows - start;
    if (num <= 1) return;

    /* Shift text grid lines */
    memmove(grid->chars[start], grid->chars[start + 1], (size_t)(num - 1) * CONSOLE_MAX_COLS);
    memmove(grid->fg[start], grid->fg[start + 1], (size_t)(num - 1) * CONSOLE_MAX_COLS);
    memmove(grid->bg[start], grid->bg[start + 1], (size_t)(num - 1) * CONSOLE_MAX_COLS);

    memset(grid->chars[start + num - 1], ' ', (size_t)grid->cols);
    memset(grid->fg[start + num - 1], fg, (size_t)grid->cols);
    memset(grid->bg[start + num - 1], bg, (size_t)grid->cols);
}

void console_grid_move_cursor(ConsoleGrid *grid, int x, int y)
{
    if (grid == NULL) return;
    if (x >= 0 && x < grid->cols) grid->cursor_x = x;
    if (y >= 0 && y < grid->rows) grid->cursor_y = y;
}

void console_grid_write_char(ConsoleGrid *grid, char c, unsigned char fg, unsigned char bg)
{
    if (grid == NULL) return;
    int start = grid->scroll_start;
    int num = grid->scroll_lines;
    if (start < 0) start = 0;
    if (num <= 0) num = grid->rows - start;
    if (start + num > grid->rows) num = grid->rows - start;
    int scroll_end = start + num;
    int max_y = (grid->cursor_y >= start && grid->cursor_y < scroll_end) ? scroll_end : grid->rows;

    if (c == '\n') {
        grid->cursor_x = 0;
        grid->cursor_y++;
        if (grid->cursor_y >= max_y) {
            if (max_y == scroll_end) {
                console_grid_scroll(grid, fg, bg);
                grid->cursor_y = scroll_end - 1;
            } else {
                int old_start = grid->scroll_start;
                int old_lines = grid->scroll_lines;
                grid->scroll_start = 0;
                grid->scroll_lines = grid->rows;
                console_grid_scroll(grid, fg, bg);
                grid->scroll_start = old_start;
                grid->scroll_lines = old_lines;
                grid->cursor_y = grid->rows - 1;
            }
        }
        return;
    }
    if (c == '\r') {
        grid->cursor_x = 0;
        return;
    }
    if (c == '\t') {
        grid->cursor_x = (grid->cursor_x + 8) & ~7;
        if (grid->cursor_x >= grid->cols) {
            grid->cursor_x = 0;
            grid->cursor_y++;
            if (grid->cursor_y >= max_y) {
                if (max_y == scroll_end) {
                    console_grid_scroll(grid, fg, bg);
                    grid->cursor_y = scroll_end - 1;
                } else {
                    int old_start = grid->scroll_start;
                    int old_lines = grid->scroll_lines;
                    grid->scroll_start = 0;
                    grid->scroll_lines = grid->rows;
                    console_grid_scroll(grid, fg, bg);
                    grid->scroll_start = old_start;
                    grid->scroll_lines = old_lines;
                    grid->cursor_y = grid->rows - 1;
                }
            }
        }
        return;
    }
    if (c == '\b') {
        if (grid->cursor_x > 0) {
            grid->cursor_x--;
        }
        return;
    }

    if (grid->cursor_x >= 0 && grid->cursor_x < grid->cols && grid->cursor_y >= 0 && grid->cursor_y < grid->rows) {
        grid->chars[grid->cursor_y][grid->cursor_x] = c;
        grid->fg[grid->cursor_y][grid->cursor_x] = fg;
        grid->bg[grid->cursor_y][grid->cursor_x] = bg;
    }

    grid->cursor_x++;
    if (grid->cursor_x >= grid->cols) {
        grid->cursor_x = 0;
        grid->cursor_y++;
        if (grid->cursor_y >= max_y) {
            if (max_y == scroll_end) {
                console_grid_scroll(grid, fg, bg);
                grid->cursor_y = scroll_end - 1;
            } else {
                int old_start = grid->scroll_start;
                int old_lines = grid->scroll_lines;
                grid->scroll_start = 0;
                grid->scroll_lines = grid->rows;
                console_grid_scroll(grid, fg, bg);
                grid->scroll_start = old_start;
                grid->scroll_lines = old_lines;
                grid->cursor_y = grid->rows - 1;
            }
        }
    }
}
