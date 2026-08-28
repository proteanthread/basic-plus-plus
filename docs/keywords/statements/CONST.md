# `CONST` Constant Declaration Statement

## 1. BASIC Usage and Statement Definition

The `CONST` statement declares one or more named compile-time/runtime constants with fixed immutable values. Attempting to assign a new value to a constant variable at runtime triggers Error 17 (`ERR_CANNOT_ASSIGN_TO_CONST`).

### Syntax Signatures:
```basic
CONST constname = constant_expression [, constname2 = constant_expression2 ...]
```

### Operational Rules:
- **Immutability**: Once defined, the symbol cannot be altered by `LET`, `INPUT`, `READ`, or `FOR`.
- **Expression Evaluation**: The initial value can be a literal or a static expression computed from literals and previously declared constants.
- **Type Invariant**: The constant inherits the type from its literal or type suffix (`%`, `!`, `#`, `$`, `&`).
- **Scope**: Subject to procedure-local or module-global scoping rules.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Constant Types Supported | Scope |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | N/A | Not supported |
| **QuickBASIC / QBASIC** | `CONST NAME = value` | Numeric and String | Local/Global |
| **Visual Basic** | `Const NAME As Type = value` | All primitive types | Public/Private |
| **BASIC++ (Master)** | `CONST NAME = value` | Number, String, Typed | Local/Global |

---

## 3. Examples

```basic
10 CONST PI# = 3.141592653589793
20 CONST MAX_USERS% = 256, APP_NAME$ = "BASIC++ Core Engine"
30 PRINT APP_NAME$; " - Max capacity: "; MAX_USERS%
40 RADIUS = 10
50 AREA = PI# * RADIUS * RADIUS
60 PRINT "Area: "; AREA
```
