# `STATIC` Static Variable Declaration Statement

## 1. BASIC Usage and Statement Definition

The `STATIC` statement declares local variables within a `SUB` or `FUNCTION` procedure whose values persist across multiple invocations, preserving state rather than re-initializing on each procedure entry.

### Syntax Signatures:
```basic
STATIC var1 [AS Type] [, var2 [AS Type] ...]
STATIC ArrayName(dim1 [, dim2 ...]) [AS Type]
```

### Operational Rules:
- **Persistence**: Unlike standard `LOCAL` variables which are destroyed upon procedure return, `STATIC` variables retain their assigned values between procedure calls.
- **Initialization**: Initialized to zero (or empty string) only on the very first invocation of the containing procedure.
- **Scope**: Local to the procedure in which they are declared; inaccessible to callers or other procedures.
- **Array Support**: Static arrays are allocated on first entry and preserved across subsequent calls.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Preservation Scope | Array Support |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* (All variables global) | N/A | N/A |
| **QuickBASIC / QBASIC** | `STATIC var1, var2` | Procedure-local | Yes |
| **Visual Basic** | `Static var1 As Integer` | Procedure-local | Yes |
| **BASIC++ (Master)** | `STATIC var1 [AS Type]` | Procedure-local | Yes |

---

## 3. Examples

```basic
10 SUB GetCounter()
20   STATIC Count%
30   Count% = Count% + 1
40   PRINT "Invocation count: "; Count%
50 END SUB
60 CALL GetCounter()
70 CALL GetCounter()
80 CALL GetCounter()
```
Output:
```
Invocation count: 1
Invocation count: 2
Invocation count: 3
```
