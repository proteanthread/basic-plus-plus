# `OPTION` Compiler and Runtime Configuration Statement

## 1. BASIC Usage and Keyword Definition

The `OPTION` statement sets global compiler, dialect, array indexing, and runtime execution flags for the active BASIC++ program.

### Syntax Signatures:
```basic
OPTION BASE 0 | 1
OPTION EXPLICIT
OPTION DIALECT dialect_name$
OPTION STRICT
```

### Operational Rules:
- **`OPTION BASE 0 | 1`**: Sets default lowest array index subscript to 0 or 1. Must appear before any arrays are dimensioned (`DIM`). Changing base after array allocation raises Error 10 (`ERR_DUPLICATE_DEFINITION`).
- **`OPTION EXPLICIT`**: Mandates that all variables must be explicitly declared (`DIM`, `VAR`, `CONST`, `STATIC`) prior to assignment.
- **`OPTION DIALECT`**: Configures dialect compatibility layer (`"GWBASIC"`, `"QBASIC"`, `"SUPERBASIC"`, `"ECMA116"`).
- **`OPTION STRICT`**: Enables type safety checking and disallows implicit narrowing conversions.

---

## 2. Code Examples

```basic
10 OPTION BASE 1 : REM Set 1-based indexing for matrix arrays
20 OPTION EXPLICIT : REM Require variable declaration
30 DIM ARR(5) AS INTEGER
40 PRINT "Lower bound: "; LBOUND(ARR); " Upper bound: "; UBOUND(ARR)
```
