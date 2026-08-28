# `FUNCTION` Multi-Line Function Definition Statement Block

## 1. BASIC Usage and Keyword Definition

Defines a modular, multi-line user function with local variable scope.

### Syntax Signatures:
```basic
FUNCTION FuncName [(param1 [, param2...])]
  [statements]
  FuncName = return_value
END FUNCTION
```

### Error Handling & Boundary Conditions:
- **Error 37 (ERR_DUPLICATE_DEFINITION)**: Function already defined with different signature.

### Operational Notes:
- Supports recursion and local variable isolation.

---

## 2. Code Examples

```basic
10 PRINT Area(5, 10)
20 FUNCTION Area(W, H)
30   Area = W * H
40 END FUNCTION
```
