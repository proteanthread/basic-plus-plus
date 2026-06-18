=========================================================
 BASIC++ TUI (Text User Interface) Programming Guide
=========================================================

 Contents
 --------
 1. Introduction
 2. The VCON Buffer
 3. Cursor Control (CURSOR ON/OFF)
 4. Double Buffering (SCREEN LOCK/UNLOCK)
 5. Scroll Regions (VIEW PRINT)
 6. Color Commands (COLOR, INK, PAPER)
 7. Arrow Key & Special Key Input
 8. Mouse Support (MOUSE ON/OFF, MOUSE())
 9. Screen Save/Restore (SCREENSAVE/SCREENRESTORE)
 10. Box Drawing Helpers
 11. Windowed Dialogs
 12. Pull-Down Menus
 13. Multi-Window Desktop
 14. Text Selection & Clipboard
 15. Full-Screen Text Editor
 16. Dual-Pane File Manager
 17. Complete TUI Desktop
 18. Tips & Best Practices

=========================================================
 1. Introduction
=========================================================

BASIC++ provides a complete set of TUI (Text User
Interface) commands for building text-based applications
directly in the terminal. These commands are ALWAYS
available -- they are not gated behind any dialect mode
or special TUI flag. You can use them in any program,
from simple scripts to full desktop environments.

TUI applications treat the terminal as an 80x25
character canvas. You position text with LOCATE, set
colors with COLOR/INK/PAPER, draw boxes with CHR$()
box-drawing characters, and respond to keyboard and
mouse events via INKEY$ and MOUSE().

Classic systems that inspired this design:
 - MS-DOS EDIT / QBasic IDE
 - Borland Turbo Vision
 - Midnight Commander
 - Norton Commander
 - 4DOS Desktop
 - Freemacs / VI for DOS

=========================================================
 2. The VCON Buffer
=========================================================

BASIC++ maintains a Virtual Console (VCON) buffer: a
25-row x 80-column grid that mirrors what is displayed
on the terminal. Every character and its color attribute
are stored in this buffer.

Key properties:
 - Characters: rt->vcon_chars[25][80]
 - Colors:     rt->vcon_colors[25][80]
 - Color byte: (background << 4) | foreground
   (GW-BASIC 16-color palette, 0-15)

All output commands (PRINT, LOCATE, CLS, COLOR, INK,
PAPER) update the VCON buffer AND the terminal in sync.
The SCREEN() function reads back from the VCON buffer:

  SCREEN(row, col)     ' returns character code
  SCREEN(row, col, 1)  ' returns color attribute

=========================================================
 3. Cursor Control (CURSOR ON/OFF)
=========================================================

Syntax:
  CURSOR ON          ' show the cursor
  CURSOR OFF         ' hide the cursor
  CURSOR             ' toggle visibility

The cursor is visible by default. Hide it for clean
TUI rendering where the blinking cursor would distract:

  10 CURSOR OFF
  20 CLS
  30 ' ... draw your TUI ...
  40 ' ... when exiting: ...
  50 CURSOR ON

ANSI sequences used:
  Show:  ESC[?25h
  Hide:  ESC[?25l

NOTE: Always restore cursor visibility before your
program exits, or the user's terminal cursor will
remain hidden.

=========================================================
 4. Double Buffering (SCREEN LOCK/UNLOCK)
=========================================================

Syntax:
  SCREEN LOCK        ' begin buffered rendering
  SCREEN UNLOCK      ' flush buffer to terminal

When SCREEN LOCK is active, all PRINT/LOCATE/COLOR
output writes to the VCON buffer only -- nothing is
sent to the terminal. When SCREEN UNLOCK is called,
the entire VCON buffer is rendered to the terminal in
one efficient batch, eliminating flicker.

Example -- flicker-free full-screen redraw:

  100 SCREEN LOCK
  110 CLS
  120 COLOR 15, 1
  130 LOCATE 1, 1
  140 PRINT STRING$(80, " ");
  150 LOCATE 1, 30
  160 PRINT "My Application";
  170 COLOR 7, 0
  180 FOR R = 2 TO 24
  190   LOCATE R, 1
  200   PRINT STRING$(80, " ");
  210 NEXT R
  220 COLOR 0, 7
  230 LOCATE 25, 1
  240 PRINT " F1=Help  F10=Menu  ESC=Quit";
  250 PRINT STRING$(52, " ");
  260 SCREEN UNLOCK

Without SCREEN LOCK/UNLOCK, each LOCATE+PRINT would
individually update the terminal, causing visible
redraw flicker. With it, the user sees one instant
frame change.

How it works internally:
 - SCREEN LOCK sets rt->screen_locked = 1
 - All putchar/printf stdout calls are suppressed
 - VCON buffer writes (vcon_putc, vcon_puts) continue
 - SCREEN UNLOCK calls vcon_flush() which:
   1. Hides cursor temporarily
   2. Iterates all 25 rows, emitting ANSI position
      and color codes with character data
   3. Restores cursor position and visibility
   4. Flushes stdout in one write

=========================================================
 5. Scroll Regions (VIEW PRINT)
=========================================================

Syntax:
  VIEW PRINT top TO bottom  ' set scroll region
  VIEW PRINT                ' reset to full screen

VIEW PRINT defines a text scroll region. When PRINT
outputs past the bottom of the region, only the rows
within that region scroll -- everything outside is
fixed. This is essential for TUI layouts with a fixed
title bar and status bar.

Example -- fixed header and footer:

  10 REM Draw title bar
  20 COLOR 15, 1
  30 LOCATE 1, 1
  40 PRINT STRING$(80, " ");
  50 LOCATE 1, 30
  60 PRINT "BASIC++ Desktop";
  70 REM Draw status bar
  80 COLOR 0, 7
  90 LOCATE 25, 1
  100 PRINT "Ready";
  110 PRINT STRING$(75, " ");
  120 REM Set scroll region between bars
  130 COLOR 7, 0
  140 VIEW PRINT 2 TO 24
  150 REM Now PRINT only scrolls rows 2-24
  160 FOR I = 1 TO 50
  170   PRINT "Line"; I
  180 NEXT I
  190 REM Reset scroll region
  200 VIEW PRINT

The VIEW PRINT boundaries are enforced by:
 - vcon_putc() newline handling
 - vcon_scroll() only scrolls within the region
 - Cursor row clamped to view_print_bottom

=========================================================
 6. Color Commands (COLOR, INK, PAPER)
=========================================================

GW-BASIC style:
  COLOR foreground, background
  COLOR foreground

  Foreground: 0-15 (16 colors)
  Background: 0-7  (8 colors, or 0-15 with bright bg)

Sinclair style:
  INK n      ' foreground 0-7
  PAPER n    ' background 0-7
  BRIGHT n   ' 0=normal, 1=bright (bold)
  FLASH n    ' 0=off, 1=blink
  INVERSE n  ' 0=normal, 1=reverse video

INK and PAPER automatically sync with the VCON color
attribute, so SCREEN(row,col,1) returns the correct
combined color after using INK/PAPER.

GW-BASIC 16-color palette:
  0 = Black       8 = Dark Gray
  1 = Blue        9 = Light Blue
  2 = Green      10 = Light Green
  3 = Cyan       11 = Light Cyan
  4 = Red        12 = Light Red
  5 = Magenta    13 = Light Magenta
  6 = Brown      14 = Yellow
  7 = White      15 = Bright White

=========================================================
 7. Arrow Key & Special Key Input
=========================================================

BASIC++ enhances INKEY$ to automatically detect and
translate ANSI escape sequences into GW-BASIC-compatible
extended key codes.

On Linux, arrow keys send multi-byte ANSI sequences
(e.g., ESC [ A for Up arrow). BASIC++ internally
parses these sequences and returns a single integer:

  Key         Return Value    Scan Code
  --------    ------------    ---------
  Up          0x100 | 72      72
  Down        0x100 | 80      80
  Left        0x100 | 75      75
  Right       0x100 | 77      77
  Home        0x100 | 71      71
  End         0x100 | 79      79
  Page Up     0x100 | 73      73
  Page Down   0x100 | 81      81
  Insert      0x100 | 82      82
  Delete      0x100 | 83      83
  F1          0x100 | 59      59
  F2          0x100 | 60      60
  F3          0x100 | 61      61
  F4          0x100 | 62      62
  F5          0x100 | 63      63
  F6-F12      0x100 | 64-134  64-134
  ESC         27              --

On Windows, _getch() handles extended keys natively
with the 0/0xE0 prefix convention.

Usage pattern for INKEY$:

  100 K = INKEY$
  110 IF K = 0 THEN 100           ' no key
  120 IF K = 27 THEN END          ' ESC = quit
  130 IF K = 13 THEN GOSUB 500    ' Enter
  140 IF K = (256 + 72) THEN GOSUB 600  ' Up
  150 IF K = (256 + 80) THEN GOSUB 700  ' Down
  160 IF K = (256 + 75) THEN GOSUB 800  ' Left
  170 IF K = (256 + 77) THEN GOSUB 900  ' Right
  180 GOTO 100

The traditional 3-call pattern still works:

  100 A$ = INKEY$
  110 IF A$ = "" THEN 100
  120 IF LEN(A$) = 2 THEN K = ASC(RIGHT$(A$,1))
  130 REM K now holds the scan code

=========================================================
 8. Mouse Support (MOUSE ON/OFF, MOUSE())
=========================================================

Syntax:
  MOUSE ON           ' enable ANSI mouse tracking
  MOUSE OFF          ' disable mouse tracking

  MOUSE(0)           ' button state:
                     '   0=none, 1=left, 2=right, 4=middle
  MOUSE(1)           ' mouse column (1-based)
  MOUSE(2)           ' mouse row (1-based)
  MOUSE(3)           ' mouse enabled flag (0/1)

Event-driven mouse handling:
  ON MOUSE GOSUB line   ' set mouse event handler
  MOUSE ON              ' start receiving events

  (The ON MOUSE GOSUB handler fires via the standard
   event polling loop in exec.c, Tier 2.5.)

Example -- click detection:

  10 MOUSE ON
  20 CURSOR OFF
  30 CLS
  40 LOCATE 12, 30
  50 PRINT "Click anywhere!"
  60 REM Main loop
  70 K = INKEY$
  80 IF K = 27 THEN 200
  90 B = MOUSE(0)
  100 IF B = 0 THEN 70
  110 X = MOUSE(1)
  120 Y = MOUSE(2)
  130 LOCATE Y, X
  140 PRINT "*";
  150 GOTO 70
  200 MOUSE OFF
  210 CURSOR ON
  220 CLS

How ANSI mouse tracking works:
 - MOUSE ON emits ESC[?1000h (basic) + ESC[?1006h (SGR)
 - Terminal sends ESC[<btn;col;rowM on click
 - MOUSE OFF emits ESC[?1006l + ESC[?1000l

NOTE: Mouse tracking requires a terminal that supports
ANSI mouse reporting (xterm, Windows Terminal, iTerm2,
most modern terminals). Legacy terminals may not support
mouse input.

=========================================================
 9. Screen Save/Restore (SCREENSAVE/SCREENRESTORE)
=========================================================

Syntax:
  SCREENSAVE x1, y1, x2, y2, array()
  SCREENRESTORE array()

SCREENSAVE captures a rectangular region of the VCON
buffer into a DIM'd array. SCREENRESTORE puts it back.
This is the foundation for popup dialogs and windows.

Array format:
  Element 0 = width
  Element 1 = height
  Element 2 = x1 (original column)
  Element 3 = y1 (original row)
  Elements 4+ = (color << 8) | character

Example -- popup dialog with save/restore:

  100 REM Save the area behind the dialog
  110 DIM B(500)
  120 SCREENSAVE 20, 8, 60, 16, B()
  130 REM Draw dialog box
  140 COLOR 15, 4
  150 FOR R = 8 TO 16
  160   LOCATE R, 20
  170   PRINT STRING$(41, " ");
  180 NEXT R
  190 LOCATE 8, 20
  200 PRINT CHR$(218); STRING$(39, CHR$(196)); CHR$(191);
  210 FOR R = 9 TO 15
  220   LOCATE R, 20
  230   PRINT CHR$(179); STRING$(39, " "); CHR$(179);
  240 NEXT R
  250 LOCATE 16, 20
  260 PRINT CHR$(192); STRING$(39, CHR$(196)); CHR$(217);
  270 LOCATE 10, 30
  280 PRINT "Are you sure? (Y/N)"
  290 REM Wait for response
  300 K$ = INKEY$
  310 IF K$ = "" THEN 300
  320 REM Restore background
  330 SCREENRESTORE B()

The SCREENSAVE buffer stores both character and color
data, so the restore perfectly recreates the original
screen content with its colors.

=========================================================
 10. Box Drawing Helpers
=========================================================

BASIC++ uses Unicode/CP437 box-drawing characters via
CHR$():

  Single-line box:
  +----+----+------+-------+
  | Char | Code | CHR$() | Name    |
  +------+------+--------+---------+
  |  +   | 218  | CHR$(218) | Top-left     |
  |  +   | 191  | CHR$(191) | Top-right    |
  |  +   | 192  | CHR$(192) | Bot-left     |
  |  +   | 217  | CHR$(217) | Bot-right    |
  |  -   | 196  | CHR$(196) | Horizontal   |
  |  |   | 179  | CHR$(179) | Vertical     |
  |  +   | 195  | CHR$(195) | Left-T       |
  |  +   | 180  | CHR$(180) | Right-T      |
  |  +   | 194  | CHR$(194) | Top-T        |
  |  +   | 193  | CHR$(193) | Bot-T        |
  |  +   | 197  | CHR$(197) | Cross        |
  +------+------+--------+---------+

  Double-line box:
  +------+------+--------+---------+
  |  +   | 201  | CHR$(201) | Top-left     |
  |  +   | 187  | CHR$(187) | Top-right    |
  |  +   | 200  | CHR$(200) | Bot-left     |
  |  +   | 188  | CHR$(188) | Bot-right    |
  |  =   | 205  | CHR$(205) | Horizontal   |
  |  |   | 186  | CHR$(186) | Vertical     |
  +------+------+--------+---------+

Subroutine to draw a box (single-line):

  5000 REM DRAWBOX X1,Y1,X2,Y2
  5010 LOCATE Y1, X1
  5020 PRINT CHR$(218);
  5030 FOR I = X1+1 TO X2-1
  5040   PRINT CHR$(196);
  5050 NEXT I
  5060 PRINT CHR$(191);
  5070 FOR R = Y1+1 TO Y2-1
  5080   LOCATE R, X1
  5090   PRINT CHR$(179);
  5100   LOCATE R, X2
  5110   PRINT CHR$(179);
  5120 NEXT R
  5130 LOCATE Y2, X1
  5140 PRINT CHR$(192);
  5150 FOR I = X1+1 TO X2-1
  5160   PRINT CHR$(196);
  5170 NEXT I
  5180 PRINT CHR$(217);
  5190 RETURN

Usage:
  10 X1 = 10: Y1 = 5: X2 = 70: Y2 = 20
  20 GOSUB 5000

=========================================================
 11. Windowed Dialogs
=========================================================

A windowed dialog is a popup that:
 1. Saves the screen area behind it
 2. Draws a bordered box with title
 3. Captures input
 4. Restores the background on close

Complete dialog subroutine:

  6000 REM DIALOG: DT$=title, DX1,DY1,DX2,DY2=bounds
  6010 REM Returns DR$=result
  6020 DIM DB(2000)
  6030 SCREENSAVE DX1, DY1, DX2, DY2, DB()
  6040 COLOR 15, 1
  6050 REM Fill background
  6060 FOR R = DY1 TO DY2
  6070   LOCATE R, DX1
  6080   PRINT STRING$(DX2-DX1+1, " ");
  6090 NEXT R
  6100 REM Draw border
  6110 LOCATE DY1, DX1
  6120 PRINT CHR$(218);
  6130 FOR I = DX1+1 TO DX2-1
  6140   PRINT CHR$(196);
  6150 NEXT I
  6160 PRINT CHR$(191);
  6170 FOR R = DY1+1 TO DY2-1
  6180   LOCATE R, DX1
  6190   PRINT CHR$(179);
  6200   LOCATE R, DX2
  6210   PRINT CHR$(179);
  6220 NEXT R
  6230 LOCATE DY2, DX1
  6240 PRINT CHR$(192);
  6250 FOR I = DX1+1 TO DX2-1
  6260   PRINT CHR$(196);
  6270 NEXT I
  6280 PRINT CHR$(217);
  6290 REM Title
  6300 LOCATE DY1, DX1+2
  6310 PRINT " "; DT$; " ";
  6320 REM Input field
  6330 LOCATE DY1+2, DX1+2
  6340 PRINT "Enter value: ";
  6350 CURSOR ON
  6360 LINE INPUT DR$
  6370 CURSOR OFF
  6380 REM Restore background
  6390 SCREENRESTORE DB()
  6400 RETURN

Usage:
  10 CURSOR OFF
  20 CLS
  30 PRINT "Main screen content..."
  40 DT$ = "Input Dialog"
  50 DX1 = 15: DY1 = 8: DX2 = 65: DY2 = 14
  60 GOSUB 6000
  70 PRINT "You entered: "; DR$

=========================================================
 12. Pull-Down Menus
=========================================================

A pull-down menu system consists of:
 1. A menu bar at row 1
 2. Drop-down panels that appear on activation
 3. Arrow key navigation
 4. ESC to dismiss

Complete menu bar example:

  100 CURSOR OFF
  110 CLS
  120 REM Draw menu bar
  130 COLOR 0, 7
  140 LOCATE 1, 1
  150 PRINT STRING$(80, " ");
  160 LOCATE 1, 2
  170 PRINT " File  Edit  View  Help ";
  180 COLOR 7, 0
  190 REM Set work area
  200 VIEW PRINT 2 TO 24
  210 REM Status bar
  220 COLOR 0, 3
  230 LOCATE 25, 1
  240 PRINT " F10=Menu  ESC=Quit";
  250 PRINT STRING$(61, " ");
  260 COLOR 7, 0
  270 REM Main event loop
  280 K = INKEY$
  290 IF K = 0 THEN 280
  300 IF K = 27 THEN 900      ' ESC=quit
  310 IF K = (256+68) THEN GOSUB 500  ' F10=menu
  320 GOTO 280

  500 REM Activate menu bar
  510 MN = 1        ' current menu (1=File,2=Edit...)
  520 COLOR 15, 1   ' highlight first menu
  530 LOCATE 1, 2
  540 PRINT " File ";
  550 GOSUB 600     ' show dropdown
  560 REM Restore menu bar
  570 COLOR 0, 7
  580 LOCATE 1, 2
  590 PRINT " File  Edit  View  Help ";
  600 RETURN

  600 REM Show File dropdown
  610 DIM MB(500)
  620 SCREENSAVE 2, 2, 22, 8, MB()
  630 COLOR 0, 7
  640 FOR R = 2 TO 8
  650   LOCATE R, 2
  660   PRINT STRING$(21, " ");
  670 NEXT R
  680 REM Draw border
  690 LOCATE 2, 2
  700 PRINT CHR$(218);STRING$(19,CHR$(196));CHR$(191);
  710 LOCATE 3, 2
  720 PRINT CHR$(179);" New          ";CHR$(179);
  730 LOCATE 4, 2
  740 PRINT CHR$(179);" Open    F3   ";CHR$(179);
  750 LOCATE 5, 2
  760 PRINT CHR$(179);" Save    F2   ";CHR$(179);
  770 LOCATE 6, 2
  780 PRINT CHR$(179);" Save As      ";CHR$(179);
  790 LOCATE 7, 2
  800 PRINT CHR$(179);STRING$(19,CHR$(196));CHR$(179);
  810 LOCATE 8, 2
  820 PRINT CHR$(192);STRING$(19,CHR$(196));CHR$(217);
  830 REM Highlight first item
  840 MI = 1      ' selected item
  850 COLOR 15, 1
  860 LOCATE MI+2, 3
  870 PRINT " New          ";
  880 REM Menu navigation loop
  890 K = INKEY$
  900 IF K = 0 THEN 890
  910 IF K = 27 THEN 950       ' ESC=dismiss
  920 IF K = 13 THEN 960       ' Enter=select
  930 IF K = (256+72) AND MI > 1 THEN MI=MI-1 ' Up
  940 IF K = (256+80) AND MI < 4 THEN MI=MI+1 ' Down
  950 SCREENRESTORE MB()
  960 RETURN

=========================================================
 13. Multi-Window Desktop
=========================================================

A multi-window desktop draws overlapping rectangular
regions on the screen. Each "window" is a bordered box
with a title bar. Windows are drawn back-to-front
(painter's algorithm -- last drawn is on top).

Window data structure (using arrays):

  10 REM Window properties: X1,Y1,X2,Y2,Title
  20 DIM WX1(8), WY1(8), WX2(8), WY2(8)
  30 DIM WT$(8)
  40 NW = 0  ' window count

  100 REM Create Window 1
  110 NW = NW + 1
  120 WX1(NW) = 5: WY1(NW) = 3
  130 WX2(NW) = 40: WY2(NW) = 15
  140 WT$(NW) = "Editor"

  150 REM Create Window 2
  160 NW = NW + 1
  170 WX1(NW) = 30: WY1(NW) = 8
  180 WX2(NW) = 75: WY2(NW) = 22
  190 WT$(NW) = "Files"

  200 REM Draw all windows (back to front)
  210 SCREEN LOCK
  220 CLS
  230 FOR W = 1 TO NW
  240   GOSUB 5000  ' draw window W
  250 NEXT W
  260 SCREEN UNLOCK
  270 GOTO 270

  5000 REM DRAW WINDOW W
  5010 COLOR 14, 1
  5020 FOR R = WY1(W) TO WY2(W)
  5030   LOCATE R, WX1(W)
  5040   PRINT STRING$(WX2(W)-WX1(W)+1, " ");
  5050 NEXT R
  5060 REM Title bar
  5070 COLOR 15, 4
  5080 LOCATE WY1(W), WX1(W)
  5090 PRINT " "; WT$(W);
  5100 PRINT STRING$(WX2(W)-WX1(W)-LEN(WT$(W)), " ");
  5110 REM Border
  5120 COLOR 14, 1
  5130 LOCATE WY1(W)+1, WX1(W)
  5140 PRINT CHR$(195);
  5150 FOR I = WX1(W)+1 TO WX2(W)-1
  5160   PRINT CHR$(196);
  5170 NEXT I
  5180 PRINT CHR$(180);
  5190 FOR R = WY1(W)+1 TO WY2(W)
  5200   LOCATE R, WX1(W)
  5210   PRINT CHR$(179);
  5220   LOCATE R, WX2(W)
  5230   PRINT CHR$(179);
  5240 NEXT R
  5250 LOCATE WY2(W), WX1(W)
  5260 PRINT CHR$(192);
  5270 FOR I = WX1(W)+1 TO WX2(W)-1
  5280   PRINT CHR$(196);
  5290 NEXT I
  5300 PRINT CHR$(217);
  5310 RETURN

To bring a window to front: move it to the end of the
array and redraw all windows.

=========================================================
 14. Text Selection & Clipboard
=========================================================

Text selection and clipboard support can be implemented
using VCON buffer reads and string arrays:

Clipboard buffer:
  10 DIM CB$(100)    ' clipboard lines
  20 CBN = 0         ' clipboard line count

Block marking:
  10 REM Set mark start
  20 MR1 = CSRLIN    ' start row
  30 MC1 = POS(0)    ' start column
  40 MR2 = MR1: MC2 = MC1  ' end = start
  50 MARKING = 1

  60 REM Highlight marked region
  70 FOR R = MR1 TO MR2
  80   FOR C = MC1 TO MC2
  90     CH = SCREEN(R, C)
  100    OC = SCREEN(R, C, 1)
  110    REM Invert color
  120    FG = (OC AND 15)
  130    BG = (OC \ 16) AND 15
  140    COLOR BG, FG
  150    LOCATE R, C
  160    PRINT CHR$(CH);
  170  NEXT C
  180 NEXT R
  190 COLOR 7, 0

Copy to clipboard:
  200 CBN = 0
  210 FOR R = MR1 TO MR2
  220   CBN = CBN + 1
  230   CB$(CBN) = ""
  240   FOR C = MC1 TO MC2
  250     CB$(CBN) = CB$(CBN) + CHR$(SCREEN(R, C))
  260   NEXT C
  270 NEXT R

Paste from clipboard:
  300 FOR I = 1 TO CBN
  310   PRINT CB$(I)
  320 NEXT I

=========================================================
 15. Full-Screen Text Editor
=========================================================

A minimal MS-DOS EDIT clone using TUI commands:

  10 REM === BASIC++ Text Editor ===
  20 DIM TX$(500)         ' text buffer (500 lines)
  30 TN = 0               ' total lines
  40 CY = 1: CX = 1       ' cursor position
  50 TOP = 1               ' first visible line
  60 FN$ = ""              ' filename
  70 MODIFIED = 0
  80 CURSOR OFF

  100 REM Draw screen
  110 SCREEN LOCK
  120 COLOR 0, 7
  130 LOCATE 1, 1
  140 PRINT " File  Edit  Search";
  150 PRINT STRING$(61, " ");
  160 COLOR 0, 3
  170 LOCATE 25, 1
  180 PRINT " Ln:"; CY; " Col:"; CX;
  190 IF MODIFIED THEN PRINT " [Modified]";
  200 PRINT STRING$(50, " ");
  210 COLOR 15, 1
  220 VIEW PRINT 2 TO 24
  230 FOR R = 2 TO 24
  240   LOCATE R, 1
  250   LN = TOP + R - 2
  260   IF LN <= TN THEN
  270     PRINT LEFT$(TX$(LN) + STRING$(80," "), 80);
  280   ELSE
  290     PRINT STRING$(80, " ");
  300   END IF
  310 NEXT R
  320 LOCATE CY - TOP + 2, CX
  330 SCREEN UNLOCK
  340 CURSOR ON

  400 REM Event loop
  410 K = INKEY$
  420 IF K = 0 THEN 410
  430 CURSOR OFF
  440 IF K = 27 THEN 800           ' ESC=quit
  450 IF K = (256+72) THEN CY=CY-1 ' Up
  460 IF K = (256+80) THEN CY=CY+1 ' Down
  470 IF K = (256+75) THEN CX=CX-1 ' Left
  480 IF K = (256+77) THEN CX=CX+1 ' Right
  490 IF K = (256+71) THEN CX=1    ' Home
  500 IF K = (256+79) THEN CX=LEN(TX$(CY))+1 ' End
  510 IF K = (256+73) THEN CY=CY-23 ' PgUp
  520 IF K = (256+81) THEN CY=CY+23 ' PgDn
  530 IF K = 13 THEN GOSUB 700     ' Enter
  540 IF K = 8 THEN GOSUB 720      ' Backspace
  550 IF K >= 32 AND K <= 126 THEN GOSUB 740 ' Char
  560 REM Clamp cursor
  570 IF CY < 1 THEN CY = 1
  580 IF CY > TN + 1 THEN CY = TN + 1
  590 IF CX < 1 THEN CX = 1
  600 REM Scroll
  610 IF CY < TOP THEN TOP = CY
  620 IF CY > TOP + 22 THEN TOP = CY - 22
  630 GOTO 100

  700 REM Insert line at Enter
  710 REM (split current line at cursor)
  720 RETURN

  740 REM Insert character
  750 IF CY > TN THEN TN = CY: TX$(CY) = ""
  760 TX$(CY) = LEFT$(TX$(CY),CX-1) + CHR$(K) + MID$(TX$(CY),CX)
  770 CX = CX + 1
  780 MODIFIED = 1
  790 RETURN

  800 REM Quit
  810 VIEW PRINT
  820 CURSOR ON
  830 CLS
  840 END

=========================================================
 16. Dual-Pane File Manager
=========================================================

Midnight Commander-style dual-pane file manager:

  10 REM === BASIC++ File Manager ===
  20 CURSOR OFF
  30 CLS

  100 REM Draw frame
  110 SCREEN LOCK
  120 COLOR 0, 3
  130 LOCATE 1, 1
  140 PRINT " Left";STRING$(34, " ");"Right";
  150 PRINT STRING$(35, " ");
  160 REM Left pane border (cols 1-40)
  170 COLOR 14, 1
  180 FOR R = 2 TO 23
  190   LOCATE R, 1
  200   PRINT CHR$(179);STRING$(38, " ");CHR$(179);
  210 NEXT R
  220 REM Right pane border (cols 41-80)
  230 FOR R = 2 TO 23
  240   LOCATE R, 41
  250   PRINT CHR$(179);STRING$(38, " ");CHR$(179);
  260 NEXT R
  270 REM Bottom border
  280 LOCATE 24, 1
  290 PRINT CHR$(192);STRING$(38,CHR$(196));CHR$(193);
  300 PRINT STRING$(38,CHR$(196));CHR$(217);
  310 REM Status bar
  320 COLOR 0, 3
  330 LOCATE 25, 1
  340 PRINT " F5=Copy F6=Move F7=Mkdir F8=Delete";
  350 PRINT STRING$(44, " ");
  360 SCREEN UNLOCK
  370 REM (populate panes with directory listings)
  380 REM (arrow keys navigate, TAB switches panes)
  390 GOTO 390

=========================================================
 17. Complete TUI Desktop
=========================================================

A complete TUI desktop combines all the above:

  10 REM === BASIC++ TUI Desktop ===
  20 CURSOR OFF
  30 CLS
  40 DIM WB(5000)   ' window save buffers
  50 CLOCK.ON = 1   ' show clock

  100 REM Draw desktop background
  110 SCREEN LOCK
  120 COLOR 7, 1
  130 FOR R = 2 TO 24
  140   LOCATE R, 1
  150   PRINT STRING$(80, CHR$(176));
  160 NEXT R
  170 REM Menu bar
  180 COLOR 0, 7
  190 LOCATE 1, 1
  200 PRINT " System  Programs  Windows  Help";
  210 PRINT STRING$(48, " ");
  220 REM Taskbar
  230 COLOR 15, 3
  240 LOCATE 25, 1
  250 PRINT " [Start]";
  260 PRINT STRING$(60, " ");
  270 T$ = TIME$
  280 LOCATE 25, 72
  290 PRINT T$;
  300 SCREEN UNLOCK
  310 REM Open initial windows
  320 REM (call window-create subroutines)

  400 REM Desktop event loop
  410 K = INKEY$
  420 IF K = 27 THEN 900          ' ESC = shutdown
  430 IF K = (256+68) THEN GOSUB 500 ' F10 = menu
  440 B = MOUSE(0)
  450 IF B > 0 THEN GOSUB 600    ' mouse click
  460 REM Update clock every second
  470 IF TIME$ <> T$ THEN T$ = TIME$: LOCATE 25, 72: COLOR 15, 3: PRINT T$;
  480 GOTO 410

  500 REM Open system menu
  510 RETURN

  600 REM Handle mouse click
  610 MX = MOUSE(1): MY = MOUSE(2)
  620 IF MY = 1 THEN GOSUB 500   ' menu bar
  630 IF MY = 25 THEN GOSUB 700  ' taskbar
  640 RETURN

  700 REM Taskbar click
  710 RETURN

  900 REM Shutdown
  910 CURSOR ON
  920 MOUSE OFF
  930 VIEW PRINT
  940 CLS
  950 PRINT "Desktop closed."
  960 END

=========================================================
 18. Tips & Best Practices
=========================================================

1. ALWAYS restore cursor visibility before exiting:
     CURSOR ON

2. ALWAYS disable mouse tracking before exiting:
     MOUSE OFF

3. Use SCREEN LOCK/UNLOCK for any full-screen redraw
   to eliminate flicker.

4. Use SCREENSAVE/SCREENRESTORE for popups and dialogs
   so you can cleanly dismiss them.

5. Use VIEW PRINT to protect title bars and status bars
   from scrolling.

6. Use COLOR 0, 7 for menu bars (black on white),
   COLOR 15, 1 for highlighted items (white on blue),
   COLOR 14, 1 for window content (yellow on blue).

7. For box drawing, define constants at the top:
     TL = 218: TR = 191: BL = 192: BR = 217
     HZ = 196: VT = 179

8. Check MOUSE(3) before using mouse functions to
   verify mouse tracking is active.

9. When building complex TUI apps, use subroutines
   (GOSUB) for reusable components:
     GOSUB DrawBox
     GOSUB DrawDialog
     GOSUB DrawMenuBar
     GOSUB DrawStatusBar

10. Test your TUI on both Windows Terminal and a Linux
    terminal (xterm, gnome-terminal) to ensure ANSI
    compatibility.

=========================================================
 End of TUI Programming Guide
=========================================================
