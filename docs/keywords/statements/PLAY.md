# `PLAY` Music Macro Language Audio Playback Statement

## 1. BASIC Usage and Keyword Definition

Plays musical melodies and sound effects using Music Macro Language (MML) strings.

### Syntax Signatures:
```basic
PLAY mml_command_string$
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid MML command syntax.

### Operational Notes:
- Full support for T, O, L, C-B, #, -, ., P, MN, ML, MS, MB, MF, V commands.

---

## 2. Code Examples

```basic
10 PLAY "T120 O4 L4 C D E F G2 G2 A A A A G1"
```
