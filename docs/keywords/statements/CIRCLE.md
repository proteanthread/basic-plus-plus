# `CIRCLE` Graphics Circle and Ellipse Drawing Statement

## 1. BASIC Usage and Keyword Definition

Draws circles, ellipses, arcs, and pie slices on the active graphics screen.

### Syntax Signatures:
```basic
CIRCLE (x%, y%), radius% [, color% [, start_rad [, end_rad [, aspect]]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Radius < 0 or invalid aspect ratio.

### Operational Notes:
- Pixel-exact GW-BASIC and QuickBASIC circle rasterizer algorithm.

---

## 2. Code Examples

```basic
10 SCREEN 12 : REM 640x480 16-color
20 CIRCLE (320, 240), 100, 14 : REM Yellow circle
30 CIRCLE (320, 240), 80, 10, 0, 3.14159 : REM Top arc
```
