# BASIC++ v6.5.2 Variable Scoping & Modular Declarations

## 1. OVERVIEW

BASIC++ provides a comprehensive scoping hierarchy supporting root global declarations, block scopes, modular exports, targeted sharing, and procedure-level isolation.

## 2. SCOPING STATEMENTS

### `GLOBAL`
Declares global variables accessible across all routines and blocks:
- At the root level: `GLOBAL g1, g2 AS INTEGER` declares root global variables.
- Inside a procedure: `GLOBAL g1, g2` binds the local reference directly to the root global scope.

### `PUBLIC` and `EXPORT`
Exports variables from modules or procedures:
- `PUBLIC v1, v2` — Declares variables as public.
- `EXPORT v1, v2 [TO / FOR Routine1, Routine2]` — Exports variables to other modules or selectively exports them to target routines.

### `SHARE` and `SHARED`
Imports or targets variables across procedure boundaries:
- `SHARED v1, v2` — Imports main-program global variables into the current subroutine or function.
- `SHARE v1, v2 [WITH Routine1, Routine2]` — Selectively shares variables with specific named procedures.

### `LOCAL` and `STATIC`
- `LOCAL v1, v2` — Explicitly defines variables isolated to the current procedure frame, shadowed from caller variables.
- `STATIC v1, v2` — Retains variable state across subsequent invocations of the procedure.

## 3. SCOPE BLOCKS

The `SCOPE` statement creates an isolated execution context:

```basic
10 X = 100
20 SCOPE
30   X = 42              ' Local X
40   PRINT X             ' Prints 42
50 END SCOPE
60 PRINT X               ' Prints 100 (outer X preserved)
```

### Importing & Exporting in Scope Blocks:
- `SCOPE IMPORT var1, var2` brings outer variables into the scope block.
- `SCOPE EXPORT var1, var2` exposes scope-local variables to the outer program.

## 4. MULTI-VARIABLE AND CHAINED FOR LOOPS

BASIC++ supports flexible multi-variable parallel stepping and chained loop bounds:
- **Multi-Variable Parallel Stepping**:
  ```basic
  FOR A, B, C = 1 TO 5
    PRINT A, B, C
  NEXT
  ```
  Accepts bare `NEXT`, matching variables `NEXT A, B, C`, reversed `NEXT C, B, A`, or single variable `NEXT A`.
- **Chained Initializer**:
  ```basic
  FOR P=Q=R=1 TO 4
    PRINT P, Q, R
  NEXT P, Q, R
  ```
