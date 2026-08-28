# `LOCATE` Text Cursor Positioning Statement

## 1. BASIC Usage and Keyword Definition

Positions the text cursor at a specific (row, col) coordinate and configures hardware cursor appearance.

### Syntax Signatures:
```basic
LOCATE [row%] [, [col%] [, [cursor_flag%] [, [start_scan%] [, stop_scan%]]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Row or column outside screen dimensions.

### Operational Notes:
- Standard 80x25 and 40x25 text positioning.

---

## 2. Code Examples

```basic
10 CLS
20 LOCATE 12, 35, 1 : REM Center cursor and make visible
30 PRINT "Centered Text"
```
