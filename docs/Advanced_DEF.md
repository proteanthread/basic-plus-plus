# Advanced DEF Functions in BASIC++

**Version 4.0.1**


---

## Table of Contents

- DEF FN (Single-Line Functions)
- DEF FN with Strings
- FUNCTION Blocks (QBasic Style)
- SUB Procedures
- Scope and Variables
- Recursion
- DEF USR (Machine Code Functions)
- Practical Patterns
  - Lookup table function
  - Clamping function
  - Linear interpolation
  - Sorting subroutine
- Limitations

---

BASIC++ supports user-defined functions at three levels of complexity: single-line `DEF FN`, multi-line `FUNCTION` blocks, and `SUB` procedures.

---

## 1. DEF FN (Single-Line Functions)

The simplest form. One expression, inline:

```basic
DEF FN name(param) = expression
```

**Examples:**

```basic
DEF FN SQUARE(X) = X * X
DEF FN CUBE(X) = X * X * X
DEF FN AREA(R) = 3.14159 * R * R
DEF FN CLAMP(X) = -((X < 0) * 0 + (X > 255) * 255 + (X >= 0 AND X <= 255) * X)
DEF FN MAX(A, B) = -(A * (A >= B) + B * (B > A))
```

**Usage:**

```basic
PRINT FN SQUARE(5)           ' 25
PRINT FN AREA(10)            ' 314.159
A = FN CUBE(3)               ' 27
```

**Rules:**
- `FN` prefix is required when calling
- Parameters are single-letter (A–Z)
- The expression can use any BASIC function
- `DEF FN` functions can reference global variables
- Multiple parameters: `DEF FN ADD(A, B) = A + B`

---

## 2. DEF FN with Strings

Define string functions by appending `$` to the name:

```basic
DEF FN REVERSE$(A$) = ...    ' Not directly possible
                              ' as a single expression
```

For string functions, use `FUNCTION` blocks (see section 3).

However, `DEF FN` can return numeric results from strings:

```basic
DEF FN FIRSTCHAR(A$) = ASC(LEFT$(A$, 1))
PRINT FN FIRSTCHAR("Hello")   ' 72
```

---

## 3. FUNCTION Blocks (QBasic Style)

Multi-line functions with local variables and full logic:

```basic
FUNCTION name(param1, param2, ...)
  statements
  name = return_value
END FUNCTION
```

**Example:**

```basic
FUNCTION Factorial(N)
  IF N <= 1 THEN
    Factorial = 1
  ELSE
    Factorial = N * Factorial(N - 1)
  END IF
END FUNCTION

PRINT Factorial(5)            ' 120
```

**String functions:**

```basic
FUNCTION Reverse$(S$)
  R$ = ""
  FOR I = LEN(S$) TO 1 STEP -1
    R$ = R$ + MID$(S$, I, 1)
  NEXT I
  Reverse$ = R$
END FUNCTION

PRINT Reverse$("Hello")       ' "olleH"
```

---

## 4. SUB Procedures

Subroutines that don't return a value:

```basic
SUB name(param1, param2, ...)
  statements
END SUB

CALL name(arg1, arg2, ...)
```

**Example:**

```basic
SUB PrintBox(W, H)
  FOR R = 1 TO H
    FOR C = 1 TO W
      IF R = 1 OR R = H OR C = 1 OR C = W THEN
        PRINT "*";
      ELSE
        PRINT " ";
      END IF
    NEXT C
    PRINT
  NEXT R
END SUB

CALL PrintBox(10, 5)
```

Output:
```
**********
*        *
*        *
*        *
**********
```

---

## 5. Scope and Variables

| Type | Parameters | Globals | Return Value |
|------|-----------|---------|-------------|
| `DEF FN` | Shadow globals during call | Read/write | Expression result |
| `FUNCTION` | Local to function | Accessible | Assign to function name |
| `SUB` | Local | Accessible | None |

---

## 6. Recursion

`FUNCTION` blocks support recursion:

```basic
FUNCTION Fib(N)
  IF N <= 1 THEN
    Fib = N
  ELSE
    Fib = Fib(N - 1) + Fib(N - 2)
  END IF
END FUNCTION

FOR I = 0 TO 10
  PRINT I, Fib(I)
NEXT I
```

Stack depth is limited by `MAX_STACK_DEPTH` (default 64). Deep recursion will cause a "Stack overflow" error.

---

## 7. DEF USR (Machine Code Functions)

For compatibility with classic BASIC, `DEF USR` defines a "machine code" function that reads/writes virtual memory:

```basic
DEF USR = address
result = USR(argument)
```

In BASIC++, `USR` reads a byte from the virtual memory at the `DEF USR` address (using the current `MEMMAP`) and returns it. The argument is `POKE`d to a fixed location.

**Example:**

```basic
MEMMAP "C64"
DEF USR = &HC000       ' Routine at $C000
A = USR(0)             ' Call it
```

---

## 8. Practical Patterns

### A. Lookup table function

```basic
DEF FN DAYNAME(D) = VAL(MID$("SunMonTueWedThuFriSat", D*3-2, 3))
' (Returns numeric -- for string lookup, use FUNCTION)
```

### B. Clamping function

```basic
DEF FN CLAMP(X, LO, HI) = -(LO*(X<LO) + HI*(X>HI) + X*(X>=LO AND X<=HI))
```

### C. Linear interpolation

```basic
DEF FN LERP(A, B, T) = A + (B - A) * T
PRINT FN LERP(0, 100, 0.5)   ' 50
```

### D. Sorting subroutine

```basic
SUB BubbleSort(A(), N)
  FOR I = 1 TO N - 1
    FOR J = 1 TO N - I
      IF A(J) > A(J + 1) THEN SWAP A(J), A(J + 1)
    NEXT J
  NEXT I
END SUB
```

---

## 9. Limitations

- `DEF FN`: single expression only, no statements
- `DEF FN`: up to 26 single-letter parameters
- `FUNCTION`/`SUB`: available in BPP and QBAS dialects only
- Maximum recursion depth: 64 (configurable in `config.h`)
- `FUNCTION`/`SUB` cannot be nested (no inner functions)
- `DEF FN` cannot call `FUNCTION`/`SUB` and vice versa
