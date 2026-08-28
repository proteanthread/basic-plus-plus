# `MED` Statistical Median Value Function

## 1. BASIC Usage and Function Definition

The `MED` function computes the statistical median (middle value of a sorted distribution) across multiple numeric arguments or array elements.

### Syntax Signatures:
```basic
median = MED(val1, val2, val3 [, ...])
median = MED(array())
```

### Operational Rules:
- Sorts values and selects the central value (or average of two central values for even $N$).

---

## 2. Code Examples

```basic
10 PRINT "Median of 1, 5, 2, 8, 7 = "; MED(1, 5, 2, 8, 7) : REM Outputs 5
```
