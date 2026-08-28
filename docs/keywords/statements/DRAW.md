# `DRAW` Graphics Macro Language Drawing Statement

## 1. BASIC Usage and Keyword Definition

Interprets a Graphics Macro Language (GML) command string to draw lines, shapes, and angles.

### Syntax Signatures:
```basic
DRAW command_string$
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid GML syntax in command string.

### Operational Notes:
- Full support for U, D, L, R, E, F, G, H, M, B, N, A, TA, C, S, and X commands.

---

## 2. Code Examples

```basic
10 SCREEN 12
20 DRAW "BM320,240 U50 R50 D50 L50" : REM Draw square
```
