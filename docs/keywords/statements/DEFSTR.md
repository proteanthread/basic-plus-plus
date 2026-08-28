# `DEFSTR` Define String Default Type

## 1. BASIC Usage and Keyword Definition

The `DEFSTR` statement declares that all variable identifiers beginning with the specified letter(s) or letter ranges default to string variables (`$` / reference-counted string) when declared or referenced without an explicit `$` type suffix.

### Syntax Signatures:
```basic
DEFSTR letter_range [, letter_range2, ...]
```

### Operational Rules:
- Un-suffixed variable names matching the letter ranges will hold reference-counted string objects.
- Overridden by explicit numeric suffixes (`%`, `!`, `#`).

---

## 2. Code Examples

```basic
10 DEFSTR S
20 NAME = "BASIC++ Standard Edition" : REM Treated as string
30 PRINT "Title: "; NAME
```
