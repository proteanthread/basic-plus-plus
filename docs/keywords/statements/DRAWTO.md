# `DRAWTO` Absolute Coordinate Line Drawing Statement

## 1. BASIC Usage and Keyword Definition

Draws a line from the current graphics pen location to the specified (x,y) coordinates.

### Syntax Signatures:
```basic
DRAWTO x%, y%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen bounds.

### Operational Notes:
- Updates internal pen coordinates to (x, y).

---

## 2. Code Examples

```basic
10 SCREEN 12
20 PSET (100, 100), 15
30 DRAWTO 200, 200
```
