# `MINUTES` Current Minute Function

## 1. BASIC Usage and Function Definition

The `MINUTES` function returns the current minute component of the hour as an integer from $0$ to $59$.

### Syntax Signatures:
```basic
min% = MINUTES
min% = MINUTES()
```

### Operational Rules:
- Returns an integer in range $0 \le \text{minute} \le 59$.

---

## 2. Code Examples

```basic
10 PRINT "Current Minute: "; MINUTES
```
