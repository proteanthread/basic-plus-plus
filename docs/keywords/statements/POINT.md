# `POINT` Pixel Color Query Function

## 1. BASIC Usage and Keyword Definition

Returns the color attribute of the pixel at (x,y), or returns current graphics pen coordinates.

### Syntax Signatures:
```basic
color_val% = POINT(x%, y%)
coord = POINT(mode%)
```

### Operational Notes:
- Returns -1 if (x,y) is outside active screen viewport.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 PSET (100, 100), 14
30 PRINT "Color at (100, 100) is: "; POINT(100, 100) : REM Outputs 14
```
