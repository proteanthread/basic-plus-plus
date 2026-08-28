# `CONSOLE` Console Display Configuration Statement

## 1. BASIC Usage and Keyword Definition

Configures virtual terminal console scrolling regions and display properties.

### Syntax Signatures:
```basic
CONSOLE [start_line%] [, [num_lines%] [, [flag%]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Line count outside terminal bounds.

### Operational Notes:
- Controls ANSI scrolling regions and terminal multiplexer panes.

---

## 2. Code Examples

```basic
10 CONSOLE 1, 24, 1
20 PRINT "Console window configured."
```
