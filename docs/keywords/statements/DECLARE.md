# `DECLARE` Procedure Prototype Declaration Statement

## 1. BASIC Usage and Keyword Definition

Declares forward subprogram and function prototypes for type safety and argument validation.

### Syntax Signatures:
```basic
DECLARE SUB SubName [(param_list)]
DECLARE FUNCTION FuncName [(param_list)]
```

### Error Handling & Boundary Conditions:
- **Error 37 (ERR_DUPLICATE_DEFINITION)**: Procedure declared with conflicting signatures.

### Operational Notes:
- Mandatory in modular programs with forward references.

---

## 2. Code Examples

```basic
10 DECLARE FUNCTION AddNums#(A#, B#)
20 PRINT AddNums(10.5, 20.5)
30 FUNCTION AddNums#(A#, B#)
40   AddNums# = A# + B#
50 END FUNCTION
```
