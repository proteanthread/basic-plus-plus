# `RSET` Right-Justify String Statement

## 1. BASIC Usage and Keyword Definition

The `RSET` statement right-justifies a string expression within a target string variable or random-access file `FIELD` buffer. If the source string is shorter than the target variable length, `RSET` pads the leading positions with spaces. If the source string is longer, `RSET` truncates excess characters from the right.

### Syntax Signatures:
```basic
RSET string_variable$ = string_expression$
```

### Operational Rules:
- **Right Alignment**: Positions the source string at the right end of the buffer.
- **Leading Spaces**: Fills leftmost positions with spaces (`' '`).
- **Right Truncation**: If source length > target length, clips rightmost characters.
- **FIELD Buffer Integration**: Primarily used to right-align numeric text fields before calling `PUT`.

---

## 2. Code Examples

```basic
10 BUFFER$ = SPACE$(10) : REM 10-character field
20 RSET BUFFER$ = "42.50"
30 PRINT "["; BUFFER$; "]" : REM Outputs "[     42.50]"
```
