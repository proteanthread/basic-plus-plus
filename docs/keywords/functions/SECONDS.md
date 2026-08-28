# `SECONDS` Current Second Function

## 1. BASIC Usage and Function Definition

The `SECONDS` function returns the current second component of the minute as an integer from $0$ to $59$.

### Syntax Signatures:
```basic
sec% = SECONDS
sec% = SECONDS()
```

### Operational Rules:
- Returns an integer in range $0 \le \text{second} \le 59$.

---

## 2. Code Examples

```basic
10 PRINT "Current Second: "; SECONDS
```
