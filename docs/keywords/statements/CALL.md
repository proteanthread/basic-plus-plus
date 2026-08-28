# `CALL` Procedure & External Function Call Statement

## 1. BASIC Usage and Keyword Definition

Executes a defined SUB subprogram or external dynamic library procedure.

### Syntax Signatures:
```basic
CALL ProcedureName [(arg1 [, arg2...])]
ProcedureName arg1, arg2...
```

### Error Handling & Boundary Conditions:
- **Error 35 (ERR_SUBPROGRAM_UNDEFINED)**: Called subprogram does not exist.

### Operational Notes:
- Supports passing variables by reference or expressions by value.

---

## 2. Code Examples

```basic
10 DECLARE SUB Greet(Name$)
20 CALL Greet("BASIC++")
30 SUB Greet(N$)
40   PRINT "Hello, "; N$
50 END SUB
```
