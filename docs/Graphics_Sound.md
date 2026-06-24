# Graphics and Sound in BASIC++

**Version 4.1.1**


---

## Table of Contents

- Screen Modes
- COLOR
- Drawing Primitives
  - PSET / PRESET
  - LINE
  - CIRCLE
  - PAINT
- DRAW Command
- GET and PUT (Graphics)
- Screen Management
- Sound
  - BEEP
  - SOUND
  - PLAY (Music Macro Language)
- Graphics Buffer
- Limitations

---

BASIC++ includes a virtual graphics framebuffer and sound system compatible with GW-BASIC/QBasic graphics commands.

> **Note:** Graphics output is rendered to an internal buffer (`gfxbuf`). Display depends on the output device. In a text terminal, graphics commands operate on the buffer silently. A future GUI mode or VDev graphics device will display them visually.

---

## 1. Screen Modes

```basic
SCREEN mode
```

| Mode | Resolution | Colors | Type |
|------|-----------|--------|------|
| 0 | Text mode | 16 | 80×25 text *(default)* |
| 1 | 320×200 | 4 | CGA medium-res |
| 2 | 640×200 | 2 | CGA high-res |
| 3 | 720×348 | 2 | Hercules |
| 4 | 320×200 | 4 | EGA (CGA compat) |
| 5 | 320×200 | 4 | EGA (CGA compat) |
| 6 | 640×200 | 2 | EGA (CGA compat) |
| 7 | 320×200 | 16 | EGA lo-res |
| 8 | 640×200 | 16 | EGA hi-res |
| 9 | 640×350 | 16 | EGA |
| 10 | 640×350 | 4/64 | EGA mono |
| 11 | 640×480 | 2 | VGA |
| 12 | 640×480 | 16 | VGA |
| 13 | 320×200 | 256 | MCGA/VGA |

---

## 2. COLOR

```basic
COLOR foreground [, background]
```

**Text mode colors (0–15):**

| Code | Color | Code | Color |
|------|-------|------|-------|
| 0 | Black | 8 | Dark Gray |
| 1 | Blue | 9 | Light Blue |
| 2 | Green | 10 | Light Green |
| 3 | Cyan | 11 | Light Cyan |
| 4 | Red | 12 | Light Red |
| 5 | Magenta | 13 | Light Magenta |
| 6 | Brown | 14 | Yellow |
| 7 | White | 15 | Bright White |

```basic
COLOR 14, 1         ' Yellow on blue
PRINT "Warning!"
```

---

## 3. Drawing Primitives

### PSET / PRESET

```basic
PSET (100, 50)             ' Plot at (100,50)
PSET (100, 50), 4          ' Plot in red
PRESET (100, 50)           ' Clear pixel (set to background color)
```

### LINE

```basic
LINE (x1,y1)-(x2,y2) [, color [, B|BF]]

LINE (0,0)-(319,199)              ' Diagonal line
LINE (0,0)-(319,199), 1           ' Blue line
LINE (10,10)-(100,100), 2, B      ' Green box (outline)
LINE (10,10)-(100,100), 2, BF     ' Green box (filled)
```

### CIRCLE

```basic
CIRCLE (x, y), radius [, color [, start, end [, aspect]]]

CIRCLE (160, 100), 50             ' Circle at center
CIRCLE (160, 100), 50, 4          ' Red circle
CIRCLE (160, 100), 50, 1, 0, 3.14 ' Blue semicircle
```

### PAINT

```basic
PAINT (x, y) [, fill_color [, border_color]]

PAINT (160, 100), 2, 1            ' Fill green, stop at blue
```

---

## 4. DRAW Command

`DRAW` uses a mini-language for turtle graphics:

```basic
DRAW "command_string"
```

| Command | Description |
|---------|-------------|
| `U n` | Move up n pixels |
| `D n` | Move down n pixels |
| `L n` | Move left n pixels |
| `R n` | Move right n pixels |
| `E n` | Diagonal up-right |
| `F n` | Diagonal down-right |
| `G n` | Diagonal down-left |
| `H n` | Diagonal up-left |
| `M x,y` | Move to absolute position |
| `M +x,+y` | Move relative |
| `B` | Move without drawing (prefix) |
| `N` | Return to start after drawing (prefix) |
| `A n` | Set angle (0–3, each = 90°) |
| `C n` | Set color |
| `S n` | Set scale (1–255, default 4) |

```basic
' Draw a house
DRAW "R40 E20 L40 G20 R40 BD20 R10 U10 L10 D10"
```

---

## 5. GET and PUT (Graphics)

Capture and redraw screen regions:

```basic
DIM BUF(100)
GET (x1,y1)-(x2,y2), BUF      ' Capture region
PUT (x, y), BUF [, action]    ' Draw captured region
```

**Action modes:**

| Mode | Description |
|------|-------------|
| `PSET` | Overwrite |
| `PRESET` | Inverse overwrite |
| `AND` | Bitwise AND |
| `OR` | Bitwise OR |
| `XOR` | Bitwise XOR (useful for animation) |

**Sprite animation example:**

```basic
DIM SPRITE(50)
' Draw sprite shape
LINE (0,0)-(10,10), 1, BF
GET (0,0)-(10,10), SPRITE    ' Capture it
' Animate
FOR X = 0 TO 300
  PUT (X, 100), SPRITE, XOR  ' Draw
  FOR D = 1 TO 100 : NEXT    ' Delay
  PUT (X, 100), SPRITE, XOR  ' Erase (XOR again)
NEXT X
```

---

## 6. Screen Management

| Command | Description |
|---------|-------------|
| `CLS` | Clear screen |
| `CLS 0` | Clear screen (mode 0) |
| `LOCATE row, col` | Move cursor |
| `WIDTH columns` | Set screen width (40 or 80) |
| `VIEW [PRINT top TO bottom]` | Set text viewport |
| `WINDOW [SCREEN] [(x1,y1)-(x2,y2)]` | Set coordinate system |
| `PCOPY src, dst` | Copy video page |

| Function | Description |
|----------|-------------|
| `POINT(x, y)` | Get pixel color at (x,y) |
| `POS(0)` | Current cursor column |
| `CSRLIN` | Current cursor row |

---

## 7. Sound

### BEEP

```basic
BEEP                     ' Short audible beep (800 Hz, ~0.25 sec)
```

### SOUND

```basic
SOUND frequency, duration
```

- **Frequency:** Hz (37–32767)
- **Duration:** clock ticks (18.2 ticks/second)

```basic
SOUND 440, 18          ' A4 note, ~1 second
SOUND 523, 9           ' C5 note, ~0.5 second
SOUND 0, 18            ' Silence for ~1 second
```

### PLAY (Music Macro Language)

```basic
PLAY "string"
```

| Command | Description |
|---------|-------------|
| `C D E F G A B` | Notes |
| `+` or `#` | Sharp |
| `-` | Flat |
| `P` | Rest |
| `O n` | Octave (0–6) |
| `L n` | Length (1=whole, 2=half, 4=quarter, 8=eighth) |
| `T n` | Tempo (beats per minute, 32–255) |
| `V n` | Volume (0–15) |

```basic
PLAY "O4 L4 CDEFGAB>C"         ' C major scale
PLAY "T120 O4 L8 EEGGEECCEDD"  ' Simple melody
PLAY "MF T180 O3 L4 C.E.G."    ' Staccato
```

---

## 8. Graphics Buffer

The graphics framebuffer (`gfxbuf`) stores pixel data internally. It supports:

- Multiple screen modes (320×200 to 640×480)
- Up to 256 colors per pixel
- Multiple pages (`PCOPY`)
- `GET`/`PUT` region capture

The buffer is allocated when `SCREEN` is called and freed on `SCREEN 0` or program end.

---

## 9. Limitations

- Graphics are buffered, not immediately displayed (terminal output is text-only)
- No mouse support currently
- `PALETTE` not yet implemented
- `VIEW` coordinates are approximate
- `CIRCLE` aspect ratio is approximate
- Maximum resolution limited by buffer memory
