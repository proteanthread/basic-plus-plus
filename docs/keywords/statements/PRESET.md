# `PRESET` Reset Pixel to Background Color Statement

## 1. BASIC Usage and Keyword Definition

Plots a pixel coordinate using the current background color (or specified color), erasing any existing pixel.

### Syntax Signatures:
```basic
PRESET (x%, y%) [, color%]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen bounds.

### Operational Notes:
- Convenience statement for erasing graphics points.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 PSET (100, 100), 15 : REM White pixel
30 PRESET (100, 100)     : REM Erases pixel back to background
```
