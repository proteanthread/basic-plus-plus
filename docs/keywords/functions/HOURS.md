# `HOURS` Current Hour Function

## 1. BASIC Usage and Function Definition

The `HOURS` function returns the current hour of the day as an integer from $0$ to $23$ (24-hour military clock).

### Syntax Signatures:
```basic
hr% = HOURS
hr% = HOURS()
```

### Operational Rules:
- Returns an integer in range $0 \le \text{hour} \le 23$.

---

## 2. Code Examples

```basic
10 PRINT "Current Hour: "; HOURS
```
