# `SCREEN` Display Screen Mode Configuration Statement

## 1. BASIC Usage and Keyword Definition

Initializes the video display adapter mode (e.g. Mode 0 = Text, Mode 12 = 640x480 16-color, Mode 13 = 320x200 256-color).

### Syntax Signatures:
```basic
SCREEN mode_index% [, [color_switch%] [, [active_page%] [, [visual_page%]]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Screen mode not supported on active display.

### Operational Notes:
- Delay-loads SDL2 graphics subsystem in Standard Edition.

---

## 2. Code Examples

```basic
10 SCREEN 12 : REM 640x480 16-color VGA
20 LINE (0, 0)-(639, 479), 14
30 SCREEN 0  : REM Return to text mode
```
