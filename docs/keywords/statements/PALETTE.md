# `PALETTE` Color Palette Table Reconfiguration Statement

## 1. BASIC Usage and Keyword Definition

Reconfigures the hardware color mapping table for display attributes in palette-indexed screen modes.

### Syntax Signatures:
```basic
PALETTE [attribute_index%, rgb_color_value&]
PALETTE USING array%(index%)
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Attribute index out of range.

### Operational Notes:
- Allows dynamic palette cycling and custom color schemes.

---

## 2. Code Examples

```basic
10 SCREEN 13 : REM 320x200 256-color
20 PALETTE 1, &H3F0000 : REM Remap color 1 to bright red
30 COLOR 1 : PRINT "Now Bright Red"
```
