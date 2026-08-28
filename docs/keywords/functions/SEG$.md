# `SEG$` String Segment Extraction Function

## 1. BASIC Usage and Function Definition

The `SEG$` function extracts a substring segment from character index $N$ through character index $M$ inclusive.

### Syntax Signatures:
```basic
segment$ = SEG$(source_string$, start_idx%, end_idx%)
```

### Operational Rules:
- Extracts characters between 1-based bounds `start_idx%` and `end_idx%`.
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 S$ = "ABCDEFGHIJ"
20 PRINT SEG$(S$, 3, 6) : REM Outputs "CDEF"
```
