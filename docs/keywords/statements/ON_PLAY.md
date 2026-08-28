# `ON_PLAY` Music Background Buffer Event Trapping Statement

## 1. BASIC Usage and Keyword Definition

Installs an event handler that triggers when the background music playback buffer drops below N notes.

### Syntax Signatures:
```basic
ON PLAY(note_threshold%) GOSUB {line_number% | @label}
```

### Operational Notes:
- Enables continuous background music playback in games.

---

## 2. Code Examples

```basic
10 ON PLAY(3) GOSUB 100
20 PLAY ON
30 PLAY "MB CDEFGAB"
40 DO : LOOP
100 PLAY "CDEFGAB" : RETURN
```
