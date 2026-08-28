# `DEFDBL` Define Double-Precision Default Type

## 1. BASIC Usage and Keyword Definition

The `DEFDBL` statement declares that all variable identifiers beginning with the specified letter(s) or letter ranges default to double-precision floating-point numbers (`#` / 64-bit IEEE 754 float) when declared or referenced without an explicit type suffix.

### Syntax Signatures:
```basic
DEFDBL letter_range [, letter_range2, ...]
```

### Operational Rules:
- `letter_range` can be a single letter (e.g. `DEFDBL D`) or a range separated by a hyphen (e.g. `DEFDBL A-Z`, `DEFDBL X-Z`).
- Case-insensitive: `DEFDBL a-z` is identical to `DEFDBL A-Z`.
- Suffix overrides: An explicit variable suffix (such as `X%`, `S$`, or `N!`) overrides the `DEFDBL` default.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Precision | Notes |
|---|---|---|---|
| **GW-BASIC / BASICA** | `DEFDBL letter-range` | 64-bit float | Standard Microsoft BASIC keyword |
| **QuickBASIC / QBASIC** | `DEFDBL letter-range` | 64-bit float | Standard |
| **BASIC++ (Master)** | `DEFDBL letter-range` | 64-bit double | Global variable type dispatcher |

---

## 3. Code Examples

```basic
10 DEFDBL A-Z
20 X = 1.0 / 7.0
30 PRINT "Full double precision: "; X
```
