# `COMMON` Shared Overlay Variable Statement

## 1. BASIC Usage and Keyword Definition

The `COMMON` statement designates variables that persist across program overlays when executing `CHAIN` or `RUN "file", R` statements, allowing chained programs to share variable data in memory without disk serialization.

### Syntax Signatures:
```basic
COMMON var1 [, var2, var3, ...]
COMMON SHARED var1 [, var2, ...]
```

### Operational Rules:
- **Program Overlay Lifecycle**: Variables declared in `COMMON` are retained during `var_clear_for_chain()` and transferred to the chained program.
- **Order Matching**: When chaining between programs, `COMMON` lists in both programs must match in order and type.
- **`COMMON SHARED`**: Combines `COMMON` (cross-program persistence) with `SHARED` (global accessibility inside `SUB` and `FUNCTION` blocks).

---

## 2. Code Examples

```basic
10 COMMON USER_NAME$, ACCESS_LEVEL%, USER_ID#
20 USER_NAME$ = "Alice" : ACCESS_LEVEL% = 5 : USER_ID# = 1042
30 PRINT "Passing common variables to MODULE2.BAS..."
40 CHAIN "MODULE2.BAS"
```
