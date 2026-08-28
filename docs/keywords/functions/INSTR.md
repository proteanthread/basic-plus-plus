# `INSTR` Substring Search Function

## 1. BASIC Usage and Function Definition

The `INSTR` function searches for the first occurrence of a substring within a target string, returning its 1-based character position.

### Syntax Signatures:
```basic
pos% = INSTR([start_pos%,] target_string$, search_substring$)
```

### Operational Rules:
- Returns 1-based index where `search_substring$` begins in `target_string$`.
- Returns $0$ if not found.
- If `search_substring$` is `""`, returns `start_pos%` (or 1).

---

## 2. Code Examples

```basic
10 S$ = "Quick Brown Fox"
20 P = INSTR(S$, "Brown")
30 PRINT "Found at: "; P : REM Outputs 7
```
