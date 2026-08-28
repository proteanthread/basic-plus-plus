# `SUB` Subprogram Procedure Definition Block

## 1. BASIC Usage and Keyword Definition

Defines a modular, multi-line subprogram procedure with private local variable scope.

### Syntax Signatures:
```basic
SUB SubName [(param1 [, param2...])]
  [statements]
END SUB
```

### Error Handling & Boundary Conditions:
- **Error 37 (ERR_DUPLICATE_DEFINITION)**: SUB already defined with conflicting parameters.

### Operational Notes:
- Supports EXIT SUB to return early.

---

## 2. Code Examples

```basic
10 CALL DisplayBox("Alert", 20, 5)
20 SUB DisplayBox(Title$, W%, H%)
30   PRINT "+"; STRING$(W%-2, "-"); "+"
40   PRINT "| "; Title$; TAB(W%); "|"
50   PRINT "+"; STRING$(W%-2, "-"); "+"
60 END SUB
```
