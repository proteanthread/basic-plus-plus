# `TIM` System Timer Minute Function

## 1. BASIC Usage and Function Definition

The `TIM` function returns the minute component ($0$ to $59$) of the current system wall clock time, or minutes elapsed since midnight.

### Syntax Signatures:
```basic
minute% = TIM(0)
```

### Operational Rules:
- Queries authoritative system time via `plat_time`.

---

## 2. Code Examples

```basic
10 PRINT "Current minute: "; TIM(0)
```
