# `LSET` Left-Justify String Statement

## 1. BASIC Usage and Keyword Definition

The `LSET` statement left-justifies a string expression within a target string variable or random-access file `FIELD` buffer. If the source string is shorter than the target variable length, `LSET` pads the remainder on the right with spaces. If the source string is longer, `LSET` truncates excess characters from the right.

### Syntax Signatures:
```basic
LSET string_variable$ = string_expression$
```

### Operational Rules:
- **Left Alignment**: Copies characters starting at index 1.
- **Space Padding**: Fills remaining buffer length with spaces (`' '`).
- **Right Truncation**: If source length > target length, clips rightmost characters.
- **FIELD Buffer Integration**: Primarily used to pack data fields into random-access file record buffers before calling `PUT`.

---

## 2. Code Examples

```basic
10 BUFFER$ = SPACE$(10) : REM 10-character field
20 LSET BUFFER$ = "BASIC"
30 PRINT "["; BUFFER$; "]" : REM Outputs "[BASIC     ]"
```
