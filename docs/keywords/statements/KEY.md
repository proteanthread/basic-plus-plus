# `KEY` Function Key Definition & Softkey Display Statement

## 1. BASIC Usage and Keyword Definition

Assigns macro strings to programmable function keys (F1-F10) or controls softkey display on line 25.

### Syntax Signatures:
```basic
KEY key_number%, string_expression$
KEY ON | OFF | LIST
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Key number outside range (1..10).

### Operational Notes:
- Softkey bar rendered on bottom screen row in full console modes.

---

## 2. Code Examples

```basic
10 KEY 1, "FILES" + CHR$(13)
20 KEY ON : REM Show softkeys at screen bottom
```
