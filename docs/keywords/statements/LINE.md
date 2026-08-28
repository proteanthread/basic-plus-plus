# `LINE` Graphics Line & Box Drawing Statement

## 1. BASIC Usage and Keyword Definition

Draws lines, rectangles, and solid filled boxes on the graphics screen, or reads full lines of text.

### Syntax Signatures:
```basic
LINE [[(x1%, y1%)]-(x2%, y2%) [, [color%] [, [B | BF] [, style%]]]]
LINE INPUT [;] ["prompt";] string_var$
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Coordinates outside screen resolution.

### Operational Notes:
- B draws hollow outline; BF draws solid filled box.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 LINE (50, 50)-(200, 150), 14, B  : REM Yellow rectangle
30 LINE (220, 50)-(370, 150), 10, BF : REM Filled green box
```
