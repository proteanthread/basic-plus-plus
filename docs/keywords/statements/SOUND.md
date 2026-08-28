# `SOUND` Sound Frequency Tone Generation Statement

## 1. BASIC Usage and Keyword Definition

Generates a pure audio tone of the specified frequency (37 to 32767 Hz) for a duration in clock ticks (18.2 ticks/sec).

### Syntax Signatures:
```basic
SOUND frequency_hz%, duration_ticks%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Frequency < 37 or > 32767.

### Operational Notes:
- Non-blocking when background audio subsystem is active.

---

## 2. Code Examples

```basic
10 SOUND 440, 18 : REM Play 440 Hz (A4) for 1 second
20 SOUND 880, 9  : REM Play 880 Hz (A5) for 0.5 seconds
```
