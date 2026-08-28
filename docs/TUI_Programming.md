# BASIC++ v6.5.2 TUI Programming

## 1. OVERVIEW

BASIC++ provides TUI (Text User Interface) programming facilities through the VCon device and the TUI editor multiplexer. TUI programming uses text-mode capabilities (cursor positioning, color attributes, box drawing characters) to create interactive user interfaces within the terminal.

## 2. BUILDING A TUI

A basic TUI layout uses LOCATE and COLOR for positioning and styling, and INKEY$ for non-blocking input:

```basic
10 CLS
20 COLOR 15, 1            ' White on blue
30 FOR I = 1 TO 80 : PRINT " "; : NEXT I    ' Title bar
40 LOCATE 1, 30 : PRINT "MY APPLICATION"
50 COLOR 7, 0             ' Gray on black
60 ' Draw content area
70 FOR Row = 2 TO 23
80   LOCATE Row, 1 : PRINT SPACE$(80)
90 NEXT Row
100 ' Status bar
110 COLOR 0, 7            ' Black on gray
120 LOCATE 25, 1 : PRINT SPACE$(80)
130 LOCATE 25, 2 : PRINT "F1=Help  F2=Save  F10=Quit"
```

## 3. BOX DRAWING

Use CHR$ codes for box-drawing characters:

```basic
1000 SUB DrawBox(r, c, w, h, title$)
1010   LOCATE r, c : PRINT CHR$(218);        ' Top-left corner
1020   FOR I = 1 TO w-2 : PRINT CHR$(196); : NEXT  ' Horizontal
1030   PRINT CHR$(191)                        ' Top-right corner
1040   FOR I = 1 TO h-2
1050     LOCATE r+I, c : PRINT CHR$(179);    ' Left vertical
1060     PRINT SPACE$(w-2);
1070     PRINT CHR$(179)                      ' Right vertical
1080   NEXT I
1090   LOCATE r+h-1, c : PRINT CHR$(192);   ' Bottom-left
1100   FOR I = 1 TO w-2 : PRINT CHR$(196); : NEXT
1110   PRINT CHR$(217)                        ' Bottom-right
1120   IF LEN(title$) > 0 THEN
1130     LOCATE r, c+2 : PRINT " "; title$; " "
1140   END IF
1150 END SUB
```

## 4. KEYBOARD HANDLING

Interactive TUI programs use INKEY$ in a main loop to process keystrokes without blocking:

```basic
200 WHILE 1
210   K$ = INKEY$
220   IF K$ = "" THEN GOTO 200    ' No key pressed
230   IF K$ = CHR$(27) THEN EXIT DO   ' Escape
240   IF LEN(K$) = 2 THEN
250     ScanCode = ASC(MID$(K$, 2, 1))
260     IF ScanCode = 59 THEN GOSUB 5000   ' F1
270     IF ScanCode = 60 THEN GOSUB 6000   ' F2
280     IF ScanCode = 68 THEN END           ' F10
290     IF ScanCode = 72 THEN CurRow = CurRow - 1  ' Up
300     IF ScanCode = 80 THEN CurRow = CurRow + 1  ' Down
310     IF ScanCode = 75 THEN CurCol = CurCol - 1  ' Left
320     IF ScanCode = 77 THEN CurCol = CurCol + 1  ' Right
330   END IF
340   GOSUB 9000                   ' Redraw
350 WEND
```

## 5. MENUS

A simple pull-down menu:

```basic
2000 SUB ShowMenu(r, c, items$(), count, choice)
2010   CALL DrawBox(r, c, 20, count+2, "")
2020   FOR I = 1 TO count
2030     LOCATE r+I, c+1
2040     IF I = choice THEN COLOR 0, 7 ELSE COLOR 7, 0
2050     PRINT " "; items$(I); SPACE$(17 - LEN(items$(I)))
2060   NEXT I
2070   COLOR 7, 0
2080 END SUB
```

## 6. VIEW PRINT FOR SCROLLABLE REGIONS

VIEW PRINT restricts scrolling to a specific area:

```basic
10 VIEW PRINT 3 TO 22          ' Content area scrolls
20 FOR I = 1 TO 100
30   PRINT "Line"; I           ' Scrolls within rows 3-22
40 NEXT I
50 VIEW PRINT                   ' Restore full-screen scrolling
```

Rows outside the VIEW PRINT range (1-2 for the header, 23-25 for the status bar) remain static while the content area scrolls.

## 7. DISPLAY STATEMENT

DISPLAY provides non-scrolling output for status bars and fixed elements:

```basic
10 DISPLAY 1, 1, "Title Bar Text"
20 DISPLAY 25, 1, "Status: Ready"
```

DISPLAY writes text at the specified position without moving the cursor or triggering scrolling. This is ideal for updating status indicators within a TUI layout.

## 8. THE TUI EDITOR MULTIPLEXER

The baspp standard edition includes a multi-window TUI editor (libstandard). The multiplexer manages multiple virtual terminals, each with its own VCon state. Programs can create editor windows with MUX NEW and switch between them with CHVT n.

The editor supports four editing personalities: EDIT (BASIC-aware), EDLIN (DOS-compatible), VI (modal), and WS (WordStar). See the Editing Commands guide for details.

## 9. CURSOR CONTROL

CURSOR ON / CURSOR OFF controls cursor visibility. CURSOR BLINK / CURSOR STEADY controls blinking. These are useful when drawing TUI elements — hide the cursor during screen updates to prevent flicker, then show it when the update is complete:

```basic
10 CURSOR OFF
20 ' ... draw TUI elements ...
30 CURSOR ON
40 LOCATE InputRow, InputCol
```

## 10. COLOR PALETTE

For TUI applications that need more than 16 colors, the VCon supports ANSI 256-color mode on terminals that support it. Use COLOR with values 0-255 for the foreground and 0-255 for the background on modern terminals.
