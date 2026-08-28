# `PSET` Set Graphics Pixel Coordinate Statement

## 1. BASIC Usage and Keyword Definition

Plots a single pixel at coordinate (x,y) with the specified foreground color.

### Syntax Signatures:
```basic
PSET (x%, y%) [, color%]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen bounds.

### Operational Notes:
- Updates internal pen coordinates for relative drawing.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 PSET (320, 240), 14 : REM Yellow pixel at center
```
