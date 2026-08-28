# `BEEP` Speaker Bell Tone Statement

## 1. BASIC Usage and Keyword Definition

Emits standard 800 Hz alert tones through the system speaker for a specified number of repetitions with an optional delay in seconds between beeps.

### Syntax Signatures:
```basic
BEEP
BEEP count
BEEP count, delay
```

### Parameters:
- `count` (Optional Numeric Expression): The number of times to beep the speaker. Defaults to `1` if omitted. If `0`, the statement performs a silent no-op. Must be `>= 0` (negative values raise `Error 5: Illegal function call`).
- `delay` (Optional Numeric Expression): The delay in seconds between consecutive beeps. Defaults to `1.0` seconds if omitted. Fractional seconds are supported (e.g. `0.25`, `0.5`, `1.5`). Must be `>= 0` (negative values raise `Error 5: Illegal function call`).

### Operational Notes:
- Routes through the platform audio subsystem (`platform_sound_beep`) and virtual audio devices.
- In multi-beep sequences, the delay occurs strictly **between** consecutive beeps (no delay after the final beep).

---

## 2. Code Examples

```basic
10 REM Single standard beep
20 BEEP

30 REM Beep 3 times with default 1.0 second delay
40 BEEP 3

50 REM Beep 4 times with a fast 0.25 second delay between beeps
60 BEEP 4, 0.25

70 REM Dynamic expression for count and delay
80 N = 2: D = 0.5
90 BEEP N, D
```
