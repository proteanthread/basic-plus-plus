# `DATE` / `DATE$` System Date Function & Variable

## 1. BASIC Usage and Keyword Definition

The `DATE$` built-in string variable/function returns the current system calendar date formatted as `"MM-DD-YYYY"`. In statement form, `DATE$ = "MM-DD-YYYY"` updates the virtual system date. The numeric form `DATE` returns the date packed as an integer $YYYYMMDD$.

### Syntax Signatures:
```basic
current_date$ = DATE$
current_date$ = DATE$()
numeric_date& = DATE
DATE$ = "08-15-2026" : REM Set system date
```

### Operational Rules:
- **`DATE$` Format**: Always returns a 10-character string in `MM-DD-YYYY` format.
- **`DATE` Format**: Returns numeric value formatted as $YYYYMMDD$ (e.g., $20260815$).
- **Assignment**: Setting `DATE$ = string$` updates the system date if operating under appropriate permissions (`CAP_SYS`).

---

## 2. Code Examples

```basic
10 PRINT "Today's Date: "; DATE$
20 PRINT "Numeric Date: "; DATE
```
