# BASIC++ v6.5.2 Virtual Consoles

## 1. THE VCon SUBSYSTEM

The Virtual Console (VCon) is the primary text I/O device in BASIC++. Every PRINT, INPUT, LOCATE, COLOR, CLS, and WIDTH operation passes through the VCon interface (engine/src/device/vcon.c). The VCon maintains a complete console state that is authoritative — statement handlers must read console state (cursor position, screen size, colors) from VCon rather than caching it locally.

## 2. VCon STATE

The VConContext tracks the following persistent state:

**Cursor position** — Row and column (1-based). Updated by LOCATE, character output, line feeds, carriage returns, and screen scrolling. Read by CSRLIN (row) and POS(0) (column).

**Screen dimensions** — Total rows and columns. Set by WIDTH cols and by the terminal/window size. Read by screen introspection functions.

**Color attributes** — Foreground color (0-15), background color (0-7), and border color. Set by COLOR fg, bg, border. Read by VCon query functions.

**Scroll region** — The range of rows that scroll when text reaches the bottom. By default, the entire screen scrolls. VIEW PRINT top TO bottom restricts scrolling to the specified row range.

**Cursor visibility** — Shown or hidden. LOCATE , , 0 hides the cursor. LOCATE , , 1 shows it.

**Key buffer** — A queue of keystrokes waiting to be read by INKEY$ or INPUT. The buffer holds up to 16 characters.

## 3. CONSOLE STATEMENTS

CLS clears the screen and moves the cursor to position (1, 1). CLS 0 clears the entire screen. CLS 1 clears the graphics viewport. CLS 2 clears the text viewport.

LOCATE row, col positions the cursor. LOCATE with a single argument sets the row only. LOCATE , col sets the column only. LOCATE row, col, visible sets the cursor position and visibility.

COLOR fg sets the foreground color. COLOR fg, bg sets foreground and background. COLOR fg, bg, border sets all three. Standard color codes:

| Code | Color | Code | Color |
|------|-------|------|-------|
| 0 | Black | 8 | Dark Gray |
| 1 | Blue | 9 | Light Blue |
| 2 | Green | 10 | Light Green |
| 3 | Cyan | 11 | Light Cyan |
| 4 | Red | 12 | Light Red |
| 5 | Magenta | 13 | Light Magenta |
| 6 | Brown | 14 | Yellow |
| 7 | Light Gray | 15 | White |

WIDTH cols sets the console width. WIDTH 80 and WIDTH 40 are standard. The height is set by the terminal size.

HOME moves the cursor to position (1, 1) without clearing the screen.

## 4. PRINT FORMATTING

PRINT outputs values to the console through VCon. Semicolons suppress spacing between items. Commas advance to the next print zone (every 14 columns). A trailing semicolon or comma suppresses the line feed at the end of the PRINT:

```basic
10 PRINT "A"; "B"; "C"        ' Prints: ABC
20 PRINT "A", "B", "C"        ' Prints: A             B             C
30 PRINT "No newline";         ' Cursor stays on same line
```

TAB(n) advances to column n. SPC(n) outputs n spaces. These are used within PRINT statements: `PRINT TAB(20); "CENTERED"`.

PRINT USING formats output according to a template string. See the String Handling guide for format characters.

## 5. INPUT OPERATIONS

INPUT reads from the keyboard with an optional prompt: `INPUT "Name: "; N$`. The prompt is displayed and the cursor waits for the user to type a response and press Enter.

LINE INPUT reads an entire line including commas: `LINE INPUT "Enter text: "; T$`.

INKEY$ reads a single keystroke without waiting and without echoing. Returns an empty string if no key is pressed. Extended keys (function keys, arrows) return a two-character string starting with CHR$(0).

INPUT$(n) reads exactly n characters without echoing and without waiting for Enter.

## 6. VIRTUAL TERMINAL MULTIPLEXING

The baspp standard edition supports multiple virtual console instances through the TUI editor multiplexer. Each virtual terminal (VT) has its own independent VCon state:

CHVT n switches to virtual terminal n. The new terminal becomes the active display. Each VT has its own cursor position, color attributes, and screen content.

The multiplexer renders each VT to a region of the physical terminal. Split-screen layouts display multiple VTs simultaneously.

## 7. CONSOLE TITLE AND WINDOW

TITLE "text" sets the console window title bar text.

SCREENMOVE x, y positions the console window on the desktop (on systems that support window positioning).

RESIZE cols, rows resizes the console window.

FULLSCREEN toggles full-screen mode.

ICON "path" sets the console window icon.

## 8. PLATFORM ADAPTATION

The VCon interface is platform-independent. The actual console operations are performed by plat_console.c, which implements:

- **Windows**: Win32 Console API (SetConsoleCursorPosition, SetConsoleTextAttribute, WriteConsoleOutput).
- **Linux**: termios for raw mode, ncurses for cursor addressing and color, ANSI escape sequences for formatting.
- **FreeDOS**: INT 10h BIOS calls for cursor positioning, character output, and color setting.

This separation ensures that BASIC++ programs produce identical console behavior on all supported platforms.
