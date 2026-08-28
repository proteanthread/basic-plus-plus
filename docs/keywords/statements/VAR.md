# `VAR` Variable Declaration Statement

## 1. BASIC Usage and Statement Definition

The `VAR` statement provides modern, concise local or module-level variable declaration with optional type inference or explicit type annotation.

### Syntax Signatures:
```basic
VAR varname [AS Type] [= initial_expression]
```

### Operational Rules:
- **Type Inference**: If `AS Type` is omitted, the variable's type is inferred from the type of the `initial_expression`.
- **Initialization**: Automatically initializes the variable to the evaluated value of `initial_expression` upon declaration.
- **Explicit Scoping**: When declared within a `SUB` or `FUNCTION`, the variable is local to that procedure frame.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Type Inference | Initializer Support |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | No | No |
| **QuickBASIC / QBASIC** | `DIM var AS Type` | No | No |
| **Visual Basic .NET** | `Dim var = value` | Yes | Yes |
| **BASIC++ (Master)** | `VAR var [AS Type] [= val]` | Yes | Yes |

---

## 3. Examples

```basic
10 VAR Count% = 100
20 VAR Message$ = "System initialized"
30 VAR Rate# = 1.05
40 PRINT Message$; " Count: "; Count%; " Rate: "; Rate#
```
