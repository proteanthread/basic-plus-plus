# `TIMER` System Timer Tick Function and Event Statement

## 1. BASIC Usage and Keyword Definition

Returns the number of seconds elapsed since midnight as a high-precision floating-point value, or controls timer event trapping.

### Syntax Signatures:
```basic
elapsed_seconds# = TIMER
TIMER ON | OFF | STOP
```

### Operational Notes:
- Microsecond-accurate time query via plat_time.

---

## 2. Code Examples

```basic
10 START# = TIMER
20 FOR I = 1 TO 100000 : NEXT I
30 PRINT "Elapsed: "; TIMER - START#; " seconds"
```
