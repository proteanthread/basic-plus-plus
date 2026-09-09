<!--
Title:        Subroutines_And_Functions
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/, engine/src/eval/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Subroutines, Functions, and Procedures Architecture

The authoritative specification for procedural abstraction, stack frames, parameter passing, return values, and subroutine invocation in BASIC++ v6.5.2.

---

## 1. Procedural Abstraction Models

BASIC++ supports procedural decomposition across three distinct abstraction tiers:

1. **Vintage Subroutines (`GOSUB...RETURN`)**: Minimal overhead, line-number targeted subroutines operating in caller variable scope.
2. **User-Defined Single-Line Functions (`DEF FN`)**: Lightweight arithmetic macros returning scalar numeric or string values.
3. **Structured Procedures (`SUB` and `FUNCTION`)**: Scoped procedures with named formal parameter lists, local variable frames, recursive capability, and explicit return semantics.

---

## 2. Structured Procedures (`SUB` and `FUNCTION`)

### A. Subroutine Procedures (`SUB`)
```basic
SUB ProcedureName [(param1 [AS type] [, param2 [AS type]...])]
  statements
  [EXIT SUB]
END SUB
```
- Invoked via `CALL ProcedureName(arg1, arg2)` or bare `ProcedureName arg1, arg2`.
- Operates on an isolated stack frame; outer variables are inaccessible unless explicitly declared `SHARED`.

### B. Function Procedures (`FUNCTION`)
```basic
FUNCTION FunctionName [(param1 [AS type]...)] [AS return_type]
  statements
  FunctionName = result_value
  [EXIT FUNCTION]
END FUNCTION
```
- Invoked as an expression term: `Result = FunctionName(arg1, arg2)`.
- Value is returned by assigning to the function name identifier before returning.

---

## 3. Parameter Passing and Recursion

- **Pass by Reference (Default)**: Variables passed to `SUB` or `FUNCTION` parameters are bound by reference. Modifications to the parameter modify the caller variable.
- **Pass by Value**: Enclosing an argument in parentheses forces evaluation as an expression and passes a copy by value: `CALL UpdateVar((X))`.
- **Recursion**: Procedures support recursive re-entrance. Each invocation allocates an isolated stack frame.

---

## 4. Vintage Mechanisms (`GOSUB` and `DEF FN`)

- **`GOSUB line_num` / `RETURN`**: Pushes program return address onto the call stack and branches to `line_num`.
- **`ON expression GOSUB line1, line2, ...`**: Computed branch based on 1-based index `expression`.
- **`DEF FNname(param) = expression`**: Vintage inline user-defined function.

---

## 5. Example: Recursive Factorial Function

```basic
10 REM Procedural Recursion Demo
20 PRINT "5! = "; Factorial(5)
30 END
40 
50 FUNCTION Factorial(N AS INTEGER) AS DOUBLE
60   IF N <= 1 THEN
70     Factorial = 1
80   ELSE
90     Factorial = N * Factorial(N - 1)
100  END IF
110 END FUNCTION
```
