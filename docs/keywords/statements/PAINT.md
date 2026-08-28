# `PAINT` Graphics Flood Fill Statement

## 1. BASIC Usage and Keyword Definition

Fills an enclosed graphics region with a solid color, stopping at the specified boundary border color.

### Syntax Signatures:
```basic
PAINT (x%, y%) [, [fill_color%] [, [border_color%]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Seed coordinates outside screen bounds.

### Operational Notes:
- Scanline flood fill algorithm with boundary detection.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 CIRCLE (320, 240), 50, 14 : REM Yellow boundary
30 PAINT (320, 240), 12, 14   : REM Fill inside with red
```
