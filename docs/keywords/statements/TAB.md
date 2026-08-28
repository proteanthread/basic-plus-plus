# `TAB` Print Absolute Column Alignment Function

## 1. BASIC Usage and Keyword Definition

Moves the text output cursor to the specified 1-based column position in a PRINT or LPRINT statement.

### Syntax Signatures:
```basic
PRINT TAB(column_number%)
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Column < 1 or > 255.

### Operational Notes:
- If cursor has passed column, advances to column on next line.

---

## 2. Code Examples

```basic
10 PRINT "Name"; TAB(20); "Score"; TAB(35); "Grade"
20 PRINT "Alice"; TAB(20); "95"; TAB(35); "A"
```
