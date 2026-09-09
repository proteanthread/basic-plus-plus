<!--
Title:        Virtual_Consoles
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/vcon.c, engine/src/bios/bios_console.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Virtual Consoles (VCon) Architecture

The authoritative specification for the Virtual Console (`VCon`) subsystem, terminal emulation, character cell buffers, and console device abstraction in BASIC++ v6.5.2.

---

## 1. The VCon Subsystem Architecture

The Virtual Console (`VCon`) is the primary text I/O device in BASIC++ (`engine/src/device/vcon.c`). Every console statement (`PRINT`, `INPUT`, `LOCATE`, `COLOR`, `CLS`, and `WIDTH`) interacts directly with the authoritative VCon interface:

- **Authoritative State**: VCon maintains the ground truth for console state: cursor coordinates (1-based row and column), screen dimensions (width and height), and foreground/background color attributes.
- **Double-Buffered Character Cells**: Each cell stores an 8-bit character glyph and attribute byte (foreground 0-15, background 0-7).
- **Dirty Line Optimization**: Tracks modified screen rows to minimize terminal refresh bandwidth over serial links and remote SSH connections.

---

## 2. Terminal Emulation and Escape Decoding

VCon implements robust terminal escape sequence parsing:
- **ANSI / VT100 / VT220 Support**: Decodes standard cursor movement, color attributes, and screen erasure commands.
- **Virtual Keystroke Mapping**: Translates arrow keys, function keys (`F1`-`F12`), `Home`, `End`, `PageUp`, and `PageDown` into standard BASIC `INKEY$` sequences.

---

## 3. Console Commands and Functions

- **`LOCATE row, col [, visible]`**: Sets cursor position (1-based) and toggles visibility.
- **`COLOR fg, bg`**: Sets active character colors.
- **`CLS [mode]`**: Clears the VCon display buffer and repositions cursor to `(1, 1)`.
- **`WIDTH cols [, rows]`**: Reallocates the VCon screen buffer to match target dimensions.
- **`CSRLIN`** / **`POS(0)`**: Returns current vertical row and horizontal column.

---

## 4. Example: Direct VCon Formatting

```basic
10 REM Virtual Console Demo
20 SCREEN 0 : WIDTH 80 : CLS
30 COLOR 14, 4 : LOCATE 2, 10
40 PRINT " VIRTUAL CONSOLE (VCon) ONLINE "
50 COLOR 7, 0 : LOCATE 4, 10
60 PRINT "Current row: "; CSRLIN; " Current col: "; POS(0)
```
