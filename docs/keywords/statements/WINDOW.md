# `WINDOW` Cartesian Coordinate Window Definition Statement

## 1. BASIC Usage and Keyword Definition

Defines logical Cartesian floating-point coordinates for the graphics screen, enabling mathematical graphing.

### Syntax Signatures:
```basic
WINDOW [[SCREEN] (x1#, y1#)-(x2#, y2#)]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Inverted or zero-dimension window bounds.

### Operational Notes:
- SCREEN flag places (x1, y1) at top-left; omitting SCREEN places (x1, y1) at bottom-left.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 WINDOW (-3.14, -1.0)-(3.14, 1.0) : REM Graphing coordinates
30 FOR X = -3.14 TO 3.14 STEP 0.05
40   PSET (X, SIN(X)), 14
50 NEXT X
```
