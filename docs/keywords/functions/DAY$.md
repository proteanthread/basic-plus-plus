# `DAY$` Current Day Name String Function

## 1. BASIC Usage and Function Definition

The `DAY$` function returns the current day of the week as a full English text string (e.g. `"Sunday"`, `"Monday"`, `"Tuesday"`, `"Wednesday"`, `"Thursday"`, `"Friday"`, `"Saturday"`).

### Syntax Signatures:
```basic
name$ = DAY$
name$ = DAY$()
```

### Operational Rules:
- Returns a reference-counted string containing the weekday name derived from the system clock.

---

## 2. Code Examples

```basic
10 PRINT "Today is: "; DAY$; ", "; DATE$
```
