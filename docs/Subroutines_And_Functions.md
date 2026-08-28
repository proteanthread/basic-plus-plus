# BASIC++ v6.5.2 Subroutines, Functions, and Procedures

## 1. CLASSIC SUBROUTINES (GOSUB/RETURN)

The simplest form of code reuse is the GOSUB/RETURN pair. GOSUB pushes the current execution position onto the GosubStack and jumps to the target line. RETURN pops the saved position and resumes at the next statement after the GOSUB:

```basic
100 GOSUB 1000
110 PRINT "Back from subroutine"
120 END
1000 REM --- Print Header ---
1010 PRINT "========================="
1020 PRINT "  BASIC++ PROGRAM OUTPUT"
1030 PRINT "========================="
1040 RETURN
```

The GosubStack has a depth limit of 1023 on modern builds, 63 on FreeDOS, and 31 on embedded targets. Exceeding the limit produces Error 7 (Out of memory). Each GOSUB must have a matching RETURN; issuing RETURN with an empty stack produces Error 3 (RETURN without GOSUB).

ON n GOSUB line1, line2, line3 selects a subroutine based on the value of n. If n is 1, it calls line1; if 2, line2; and so on. If n is out of range (zero, negative, or beyond the list), execution continues at the next statement.

## 2. DEF FN USER FUNCTIONS

DEF FN defines a single-expression or multi-line inline function:

```basic
10 DEF FN AREA(R) = PI * R * R
20 DEF FN HYPOTENUSE(A, B) = SQR(A^2 + B^2)
30 PRINT FN AREA(5)
40 PRINT FN HYPOTENUSE(3, 4)
```

DEF FN functions can accept up to 16 parameters. Parameters are local to the function call and do not affect variables with the same name in the main program.

Multi-line user functions are supported using `DEF FN ... FNEND`:
```basic
100 DEF FN Factorial(N)
110   IF N <= 1 THEN Factorial = 1 ELSE Factorial = N * FN Factorial(N - 1)
120 FNEND
```

DEF FN must appear in a numbered program line. The function name must start with FN and can include a type suffix: `DEF FN SQUARE%(X)` defines an integer-returning function.

## 3. SUB PROCEDURES

SUB defines a named subroutine with formal parameters, explicit passing modes (`BYVAL` / `BYREF`), and local variable isolation:

```basic
100 SUB PrintBox(width, height, char$)
110   FOR Y = 1 TO height
120     FOR X = 1 TO width
130       PRINT char$;
140     NEXT X
150     PRINT
160   NEXT Y
170 END SUB
```

### Flexible Invocation Syntax
BASIC++ supports all standard invocation styles for procedures:
- Standard CALL with parens: `CALL PrintBox(20, 5, "*")`
- CALL without parens: `CALL PrintBox 20, 5, "*"`
- Call-less invocation: `PrintBox 20, 5, "*"`
- Parenthesized call-less invocation: `PrintBox(20, 5, "*")`

Variables declared inside a SUB are local by default. They exist only during the SUB call and are destroyed when END SUB or EXIT SUB is reached.

### Scoping Statements Inside Procedures:
- `LOCAL var1, var2` — Explicitly declares local variables shadowed from caller.
- `SHARED var1, var2` — Imports main-program variables into the SUB.
- `STATIC var1, var2` — Preserves a local variable's value across multiple invocations.
- `GLOBAL var1, var2` — Binds variables directly to the root global namespace.

```basic
300 SUB Counter()
310   STATIC count
320   count = count + 1
330   PRINT "Call #"; count
340 END SUB
```

## 4. FUNCTION PROCEDURES

FUNCTION defines a named function that returns a value. The return value is assigned to the function name within the body:

```basic
100 FUNCTION AddThree(a, b, c)
110   AddThree = a + b + c
120 END FUNCTION
```

Functions can return numeric, string, or user-defined types:
- String function: `FUNCTION Greeting$(name$)`
- Numeric function: `FUNCTION Hypotenuse(a, b)`

Functions are evaluated in expressions: `res = AddThree(5, 10, 15)`.

## 5. PARAMETER PASSING: BYVAL AND BYREF

BASIC++ provides explicit parameter passing control:
- `BYVAL` — The argument expression is evaluated and passed by value. Changes made inside the procedure do not affect caller variables.
- `BYREF` — The argument variable is passed by reference. Changes made to the parameter are automatically written back to the caller variable when the procedure finishes.
- `OPTIONAL` / Default Parameters — Parameters can declare fallback default values (e.g., `prefix$ = ">> "`) if omitted at invocation time.
- **Default Mode**: Unannotated bare variables default to by-reference; literal constants and composite expressions default to by-value.

```basic
100 SUB FormatMsg(prefix$ = ">> ", msg$ = "hello", BYREF result$ = "")
110   result$ = prefix$ + msg$
120 END SUB
```

### Named Argument Passing
Arguments can be provided in any order using named parameter notation (`param_name:=value`):
```basic
200 CALL FormatMsg(prefix$:="-- ", msg$:="world", result$:=out$)
```

### Array and UDT Parameter Passing
- Arrays are passed using open parenthesis notation `arr()` and can be indexed and mutated inside the procedure.
- User-Defined Types (UDT) / Map records are passed and have their fields mutated directly (e.g. `p.x = p.x + dx`).

```basic
300 SUB DoubleArray(arr())
310   FOR i = 1 TO 3 : arr(i) = arr(i) * 2 : NEXT
320 END SUB

330 SUB MovePoint(p, dx, dy)
340   p.x = p.x + dx
350   p.y = p.y + dy
360 END SUB
```

## 6. MODULES, NAMESPACES & IMPORT

Procedures can be organized into modular namespaces with `PUBLIC` and `PRIVATE` visibility controls:

```basic
100 MODULE MathExt
110   PUBLIC FUNCTION Square(n)
120     Square = n * n
130   END FUNCTION
140   PRIVATE FUNCTION PrivHelper(n)
150     PrivHelper = n + 1
160   END FUNCTION
170 END MODULE
```

- **Qualified Invocations**: Call directly via dot syntax `MathExt.Square(12)`.
- **In-Memory Module Import**: `IMPORT MathExt` brings public exports into local scope (`Square(5)`).
- **External Source Import**: `IMPORT "path/mod_file.bas"` dynamically loads and registers routines from external files.

## 7. MULTI-VARIABLE & ADVANCED LOOP ORDERING

BASIC++ supports multiple loop control variables in both `FOR` statements and `NEXT` closures:
- Multi-assignment `FOR`: `FOR A, B, C = 1 TO 5`
- Comma-separated `NEXT`: `NEXT C, B, A` (must specify innermost loops first)
- Space-separated `NEXT`: `NEXT C B A` or `NEXT J I` for nested loops

```basic
10 FOR A, B, C = 1 TO 3
20   PRINT A, B, C
30 NEXT C B A
```

## 8. DECLARE STATEMENTS

DECLARE provides optional forward prototypes for SUB and FUNCTION procedures:

```basic
10 DECLARE SUB DoubleVal(BYREF x, BYVAL y)
20 DECLARE FUNCTION AddThree(a, b, c)
30 CALL DoubleVal(v1, v2)
40 res = AddThree(1, 2, 3)
```

## 9. EXIT SUB AND EXIT FUNCTION

- `EXIT SUB` — Immediately returns from the current subroutine frame.
- `EXIT FUNCTION` — Immediately returns from the current function frame, yielding the value assigned to the function name.

```basic
200 SUB EarlyExitSub(BYREF flag, num)
210   IF num > 10 THEN EXIT SUB
220   flag = flag + num
230 END SUB
```

## 10. STRUCTURED EXCEPTION HANDLING: WHEN EXCEPTION IN

BASIC++ implements ECMA-116 standard structured exception handling:

```basic
1000 WHEN EXCEPTION IN
1010   ERROR 5
1020 USE
1030   PRINT "Caught exception: "; ERR; " ("; ERR$; ")"
1040 END WHEN
```

Exceptions raised via runtime errors or explicit `ERROR code` are caught by the `USE` block. `ERR` and `ERR$` are populated with the error code and description.
