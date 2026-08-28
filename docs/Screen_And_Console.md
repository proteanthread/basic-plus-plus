# BASIC++ v6.5.2 Screen and Console Operations

## 1. TEXT MODE DISPLAY

SCREEN 0 selects text mode (the default). In text mode, the console is an 80-column by 25-row character grid (configurable with WIDTH). Each cell has a character (ASCII 0-255) and a color attribute (foreground 0-15, background 0-7).

WIDTH cols sets the console width. WIDTH 80 and WIDTH 40 are standard. WIDTH 132 enables wide mode on terminals that support it. The height is determined by the terminal or window.

## 2. CURSOR CONTROL

LOCATE row, col positions the cursor at the specified row and column (both 1-based). LOCATE row, col, visible also sets cursor visibility (0=hidden, 1=visible). LOCATE row, col, visible, start, end specifies the cursor shape by scan-line range (relevant for hardware cursor emulation).

HOME moves the cursor to (1, 1) without clearing the screen.

CSRLIN returns the current cursor row. POS(0) returns the current cursor column.

## 3. SCREEN CLEARING

CLS clears the entire screen and moves the cursor to (1, 1). CLS 0 clears the entire screen (text and graphics viewports). CLS 1 clears the graphics viewport only. CLS 2 clears the text viewport only (respects VIEW PRINT boundaries).

## 4. COLOR

COLOR fg sets the foreground color. COLOR fg, bg sets foreground and background. COLOR fg, bg, border sets all three (border is relevant in CGA-compatible modes).

Standard colors (0-15): 0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta, 6=Brown, 7=Light Gray, 8=Dark Gray, 9=Light Blue, 10=Light Green, 11=Light Cyan, 12=Light Red, 13=Light Magenta, 14=Yellow, 15=White.

Colors 16-31 set blinking text (foreground colors 0-15 with blink attribute). Blinking behavior depends on the terminal and platform.

FCOLOR n sets the foreground only. BCOLOR n sets the background only. These are BASIC++ convenience statements.

## 5. KEY ROW AND FUNCTION KEY DISPLAY

KEY ON displays the function key labels on the bottom line of the screen (line 25). KEY OFF hides the function key display, making line 25 available for program output. KEY LIST displays the current function key assignments.

KEY n, "string" assigns a string to function key n (1-10). Pressing the function key enters the string as if typed. KEY n, "" disables the function key.

## 6. VIEW PRINT

VIEW PRINT top TO bottom restricts the scrollable text area to the specified row range. Text output and scrolling only affect rows within this range. Rows outside the range are unaffected by scrolling:

```basic
10 VIEW PRINT 1 TO 20       ' Scrollable area: rows 1-20
20 LOCATE 21, 1 : PRINT "Status bar (does not scroll)"
30 LOCATE 1, 1
40 FOR I = 1 TO 100
50   PRINT "Line"; I
60 NEXT I
```

VIEW PRINT without arguments restores the default (entire screen scrolls).

## 7. DISPLAY STATEMENT

DISPLAY provides a non-scrolling output operation. DISPLAY row, col, text places text at the specified position without affecting the cursor position or scroll state:

```basic
10 DISPLAY 1, 1, "Score: 0"
20 DISPLAY 1, 70, TIME$
```

This is a BASIC++ extension particularly useful for game status bars and fixed-position displays.

## 8. CONSOLE WINDOW CONTROL

TITLE "text" sets the window title bar. SCREENMOVE x, y positions the window on the desktop. RESIZE cols, rows resizes the window. FULLSCREEN toggles fullscreen mode. ICON "path" sets the window icon.

These operations call through the platform layer and may have no effect on terminals that do not support them.

## 9. CURSOR STATEMENT

CURSOR ON makes the cursor visible. CURSOR OFF hides it. CURSOR BLINK enables cursor blinking. CURSOR STEADY disables blinking. These are BASIC++ extensions that provide clearer syntax than the LOCATE visibility parameter.

## 10. PAUSE AND SLEEP

PAUSE displays "Press any key to continue..." and waits for a keypress. PAUSE "custom message" displays a custom prompt.

SLEEP n pauses execution for n seconds. SLEEP with no argument waits for a keypress (equivalent to PAUSE without a message).

DELAY n pauses for n milliseconds (finer resolution than SLEEP).

## 11. SCREEN MODE QUERIES

SCREEN returns the current screen mode number when used as a function: `M = SCREEN(0)`. SCREEN(row, col) returns the ASCII code of the character at the specified position. SCREEN(row, col, 1) returns the color attribute at the specified position.
