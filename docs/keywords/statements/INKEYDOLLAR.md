# `INKEYDOLLAR` Non-Blocking Keyboard Character Input Function

## 1. BASIC Usage and Keyword Definition

Returns a single-character string from the keyboard buffer, or an empty string if no key was pressed.

### Syntax Signatures:
```basic
k$ = INKEY$
```

### Operational Notes:
- 2-byte string returned for extended function/arrow keys (CHR$(0) + code).

---

## 2. Code Examples

```basic
10 DO
20   K$ = INKEY$
30 LOOP WHILE K$ = ""
40 PRINT "Key pressed: "; K$
```
