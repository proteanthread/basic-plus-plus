# `PLOT` Graphics Coordinate Pixel Plotting Statement

## 1. BASIC Usage and Keyword Definition

Plots a single pixel coordinate on the screen (Retro Atari / Commodore style).

### Syntax Signatures:
```basic
PLOT x%, y% [, color%]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen bounds.

### Operational Notes:
- Sets graphics pen position for subsequent DRAWTO commands.

---

## 2. Code Examples

```basic
10 GRAPHICS 8
20 PLOT 100, 50
30 DRAWTO 150, 80
```
