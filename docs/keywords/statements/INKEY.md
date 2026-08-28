# `INKEY` Single Character Scan Code Input Function

## 1. BASIC Usage and Keyword Definition

Returns the numeric ASCII or scan code of the most recently pressed key, or 0 if no key is pending.

### Syntax Signatures:
```basic
code% = INKEY
```

### Operational Notes:
- Non-blocking keyboard buffer polling.

---

## 2. Code Examples

```basic
10 K% = INKEY
20 IF K% = 27 THEN PRINT "Escape pressed!"
```
