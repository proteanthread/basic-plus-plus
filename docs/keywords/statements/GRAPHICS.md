# `GRAPHICS` Retro Hardware Graphics Mode Initialization Statement

## 1. BASIC Usage and Keyword Definition

Initializes a retro hardware screen resolution profile (Atari 8-bit, Apple II, Commodore).

### Syntax Signatures:
```basic
GRAPHICS mode_index% [, [clear_flag%] [, [flags%]]]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Mode index outside supported profiles (0..15).

### Operational Notes:
- Includes split-screen text window rendering support.

---

## 2. Code Examples

```basic
10 GRAPHICS 8 : REM Atari Mode 8 (320x192 1-color)
20 COLOR 1
30 PLOT 100, 100
```
