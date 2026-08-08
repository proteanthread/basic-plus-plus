# BASIC++ v6.5.2 Complete API Reference Manual

> **Generated**: Engine Dynamic Introspection Exporter (`baspp --export-docs`)
> **Total Registered Micro-Libraries**: 116
> **Target Environments**: Desktop (`basicpp.dll`/`so`), Server (`baspp`), IoB (`libiob.so`), WAP (`wap.wasm`), Script (`bs`), Embedded (`basstub.c`), Mobile (`libbasicpp_mobile.so`)

---

### `RND`

- **Category**: Math Functions
- **Syntax**: `RND[(x)]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (non-numeric argument)

**Description**:
Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator.

---

### `INT`

- **Category**: Math Functions
- **Syntax**: `INT(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (INT expects one numeric argument)

**Description**:
Returns the largest integer less than or equal to x (floor conversion).

---

### `ABS`

- **Category**: Math Functions
- **Syntax**: `ABS(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (ABS expects one numeric argument)

**Description**:
Returns the absolute value of a numeric expression.

---

### `SQR`

- **Category**: Math Functions
- **Syntax**: `SQR(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (SQR of negative number), Error 13: Type Mismatch (SQR expects one numeric argument)

**Description**:
Returns the non-negative square root of a numeric expression x >= 0.

---

### `SIN`

- **Category**: Math Functions
- **Syntax**: `SIN(radians)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (SIN expects one numeric argument)

**Description**:
Returns the trigonometric sine of an angle given in radians.

---

### `COS`

- **Category**: Math Functions
- **Syntax**: `COS(radians)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (COS expects one numeric argument)

**Description**:
Returns the trigonometric cosine of an angle given in radians.

---

### `TAN`

- **Category**: Math Functions
- **Syntax**: `TAN(radians)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (TAN expects one numeric argument)

**Description**:
Returns the trigonometric tangent of an angle given in radians.

---

### `ATN`

- **Category**: Math Functions
- **Syntax**: `ATN(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (ATN expects one numeric argument)

**Description**:
Returns the arctangent of a numeric expression in radians.

---

### `LOG`

- **Category**: Math Functions
- **Syntax**: `LOG(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (LOG of zero or negative number), Error 13: Type Mismatch (LOG expects one numeric argument)

**Description**:
Returns the natural logarithm (base e) of a numeric expression x > 0.

---

### `EXP`

- **Category**: Math Functions
- **Syntax**: `EXP(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument)

**Description**:
Returns e raised to the power of a numeric expression x.

---

### `ACOS`

- **Category**: Math Functions
- **Syntax**: `ACOS(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (ACOS argument out of range [-1, 1]), Error 13: Type Mismatch (ACOS expects one numeric argument)

**Description**:
Returns the arccosine of x in radians for -1.0 <= x <= 1.0.

---

### `ASIN`

- **Category**: Math Functions
- **Syntax**: `ASIN(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (ASIN argument out of range [-1, 1]), Error 13: Type Mismatch (ASIN expects one numeric argument)

**Description**:
Returns the arcsine of x in radians for -1.0 <= x <= 1.0.

---

### `ATAN2`

- **Category**: Math Functions
- **Syntax**: `ATAN2(y, x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (ATAN2 expects two numeric arguments)

**Description**:
Returns the 2-argument arctangent of y and x in radians.

---

### `CEIL`

- **Category**: Math Functions
- **Syntax**: `CEIL(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (CEIL expects one numeric argument)

**Description**:
Returns the smallest integer greater than or equal to x.

---

### `CLAMP`

- **Category**: Math Functions
- **Syntax**: `CLAMP(val, min_val, max_val)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (CLAMP expects three numeric arguments)

**Description**:
Clamps a value to be within the range [min_val, max_val].

---

### `FIX`

- **Category**: Math Functions
- **Syntax**: `FIX(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (FIX expects one numeric argument)

**Description**:
Returns the truncated integer part of x (truncates towards zero).

---

### `FLOOR`

- **Category**: Math Functions
- **Syntax**: `FLOOR(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (FLOOR expects one numeric argument)

**Description**:
Returns the largest integer less than or equal to x.

---

### `LERP`

- **Category**: Math Functions
- **Syntax**: `LERP(a, b, t)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (LERP expects three numeric arguments)

**Description**:
Performs linear interpolation between a and b using weight t (a + (b - a) * t).

---

### `PI`

- **Category**: Math Functions
- **Syntax**: `PI()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (PI expects 0 arguments)

**Description**:
Returns the mathematical constant pi (~3.141592653589793).

---

### `ROUND`

- **Category**: Math Functions
- **Syntax**: `ROUND(x [, decimals])`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (ROUND expects 1 or 2 numeric arguments)

**Description**:
Rounds a numeric expression x to the specified number of decimal places (default 0).

---

### `SGN`

- **Category**: Math Functions
- **Syntax**: `SGN(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (SGN expects one numeric argument)

**Description**:
Returns the sign of x: 1 if x > 0, 0 if x = 0, -1 if x < 0.

---

### `BIN$`

- **Category**: String Functions
- **Syntax**: `BIN$(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (BIN$ expects one numeric argument)

**Description**:
Returns the binary string representation of integer x.

---

### `CHR$`

- **Category**: String Functions
- **Syntax**: `CHR$(code)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (code out of range 0-255), Error 13: Type Mismatch (CHR$ expects one numeric argument)

**Description**:
Returns a 1-character string containing the character corresponding to ASCII code (0-255).

---

### `HEX$`

- **Category**: String Functions
- **Syntax**: `HEX$(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (HEX$ expects one numeric argument)

**Description**:
Returns the hexadecimal string representation of integer x.

---

### `INSTR`

- **Category**: String Functions
- **Syntax**: `INSTR([start,] str$, match$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (start <= 0), Error 13: Type Mismatch (expects string arguments)

**Description**:
Returns the 1-based position of the first occurrence of match$ in str$. Returns 0 if not found.

---

### `LCASE$`

- **Category**: String Functions
- **Syntax**: `LCASE$(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (LCASE$ expects one string argument)

**Description**:
Returns a copy of str$ with all uppercase letters converted to lowercase.

---

### `LEFT$`

- **Category**: String Functions
- **Syntax**: `LEFT$(str$, n)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (LEFT$ expects string and numeric arguments)

**Description**:
Returns the leftmost n characters of str$.

---

### `LEN`

- **Category**: String Functions
- **Syntax**: `LEN(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (LEN expects one string argument)

**Description**:
Returns the number of characters in str$.

---

### `LTRIM$`

- **Category**: String Functions
- **Syntax**: `LTRIM$(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (LTRIM$ expects one string argument)

**Description**:
Returns a copy of str$ with leading whitespace removed.

---

### `MID$`

- **Category**: String Functions
- **Syntax**: `MID$(str$, start [, length])`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (start <= 0 or length < 0), Error 13: Type Mismatch (MID$ argument type error)

**Description**:
Returns a substring of str$ starting at 1-based index start for length characters.

---

### `OCT$`

- **Category**: String Functions
- **Syntax**: `OCT$(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (OCT$ expects one numeric argument)

**Description**:
Returns the octal string representation of integer x.

---

### `PACK$`

- **Category**: String Functions
- **Syntax**: `PACK$(fmt$, val1 [, val2, ...])`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string format)

**Description**:
Packs binary data values into a binary string according to format template fmt$.

---

### `RIGHT$`

- **Category**: String Functions
- **Syntax**: `RIGHT$(str$, n)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (RIGHT$ expects string and numeric arguments)

**Description**:
Returns the rightmost n characters of str$.

---

### `RTRIM$`

- **Category**: String Functions
- **Syntax**: `RTRIM$(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (RTRIM$ expects one string argument)

**Description**:
Returns a copy of str$ with trailing whitespace removed.

---

### `SPACE$`

- **Category**: String Functions
- **Syntax**: `SPACE$(n)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPACE$ expects one numeric argument)

**Description**:
Returns a string consisting of n space characters.

---

### `STR$`

- **Category**: String Functions
- **Syntax**: `STR$(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (STR$ expects one numeric argument)

**Description**:
Returns the string representation of numeric expression x.

---

### `STRING$`

- **Category**: String Functions
- **Syntax**: `STRING$(n, char_spec)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (STRING$ argument type error)

**Description**:
Returns a string of n repeating characters specified by ASCII code or 1st char of string.

---

### `TRIM$`

- **Category**: String Functions
- **Syntax**: `TRIM$(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (TRIM$ expects one string argument)

**Description**:
Returns a copy of str$ with both leading and trailing whitespace removed.

---

### `UCASE$`

- **Category**: String Functions
- **Syntax**: `UCASE$(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (UCASE$ expects one string argument)

**Description**:
Returns a copy of str$ with all lowercase letters converted to uppercase.

---

### `UNPACK`

- **Category**: String Functions
- **Syntax**: `UNPACK(fmt$, bin_str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string arguments)

**Description**:
Unpacks binary data from bin_str$ according to format template fmt$.

---

### `VAL`

- **Category**: String Functions
- **Syntax**: `VAL(str$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (VAL expects one string argument)

**Description**:
Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number.

---

### `BITCOUNT`

- **Category**: Bitwise Functions
- **Syntax**: `BITCOUNT(x)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (BITCOUNT expects one numeric argument)

**Description**:
Returns the number of set bits (population count) in integer x.

---

### `READBIT`

- **Category**: Bitwise Functions
- **Syntax**: `READBIT(val, bit_pos)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Returns the bit value (0 or 1) at bit_pos (0-63) of integer val.

---

### `RESETBIT`

- **Category**: Bitwise Functions
- **Syntax**: `RESETBIT(val, bit_pos)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Returns val with the bit at bit_pos (0-63) cleared to 0.

---

### `SETBIT`

- **Category**: Bitwise Functions
- **Syntax**: `SETBIT(val, bit_pos)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Returns val with the bit at bit_pos (0-63) set to 1.

---

### `SHL`

- **Category**: Bitwise Functions
- **Syntax**: `SHL(val, shift_bits)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (shift_bits < 0 or >= 64), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Performs a bitwise left shift on integer val by shift_bits.

---

### `SHR`

- **Category**: Bitwise Functions
- **Syntax**: `SHR(val, shift_bits)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (shift_bits < 0 or >= 64), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Performs a bitwise right shift on integer val by shift_bits.

---

### `TOGGLEBIT`

- **Category**: Bitwise Functions
- **Syntax**: `TOGGLEBIT(val, bit_pos)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (bit_pos out of range 0-63), Error 13: Type Mismatch (expects numeric arguments)

**Description**:
Returns val with the bit at bit_pos (0-63) inverted (toggled).

---

### `CLOCK`

- **Category**: System Functions
- **Syntax**: `CLOCK()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (CLOCK expects 0 arguments)

**Description**:
Returns the current system Unix timestamp in seconds.

---

### `CLOCK$`

- **Category**: System Functions
- **Syntax**: `CLOCK$()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (CLOCK$ expects 0 arguments)

**Description**:
Returns the current ISO 8601 formatted date/time string (YYYY-MM-DD HH:MM:SS).

---

### `DATE$`

- **Category**: System Functions
- **Syntax**: `DATE$()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (DATE$ expects 0 arguments)

**Description**:
Returns the current system date as a string (MM-DD-YYYY).

---

### `ENVIRON$`

- **Category**: System Functions
- **Syntax**: `ENVIRON$(var_name$)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (ENVIRON$ expects one string argument)

**Description**:
Returns the value of host environment variable var_name$. Returns empty string if not set.

---

### `FRE`

- **Category**: System Functions
- **Syntax**: `FRE(dummy)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 13: Type Mismatch (FRE expects 1 argument)

**Description**:
Returns the number of available free memory bytes in the VM heap.

---

### `INKEY$`

- **Category**: System Functions
- **Syntax**: `INKEY$()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (INKEY$ expects 0 arguments)

**Description**:
Reads a character non-blockingly from console buffer. Returns empty string if no key pressed.

---

### `INP`

- **Category**: System Functions
- **Syntax**: `INP(port)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (port out of bounds), Error 13: Type Mismatch (expects numeric port)

**Description**:
Reads a byte (0-255) from hardware I/O port address.

---

### `PEEK`

- **Category**: System Functions
- **Syntax**: `PEEK(addr)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address)

**Description**:
Reads a byte (0-255) from virtual memory address addr.

---

### `TICKS`

- **Category**: System Functions
- **Syntax**: `TICKS()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (TICKS expects 0 arguments)

**Description**:
Returns the number of milliseconds elapsed since system start / platform boot.

---

### `TIME$`

- **Category**: System Functions
- **Syntax**: `TIME$()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (TIME$ expects 0 arguments)

**Description**:
Returns the current system time as a string (HH:MM:SS).

---

### `TIMER`

- **Category**: System Functions
- **Syntax**: `TIMER()`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 5: Illegal Function Call (TIMER expects 0 arguments)

**Description**:
Returns the number of seconds elapsed since midnight.

---

### `GOTO`

- **Category**: Control Flow
- **Syntax**: `GOTO line_num`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 8: Undefined line number (target line does not exist), Error 2: Syntax error (missing line number)

**Description**:
Unconditionally transfers execution to the specified program line number.

---

### `GOSUB`

- **Category**: Control Flow
- **Syntax**: `GOSUB line_num`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 8: Undefined line number (subroutine target missing), Error 2: Syntax error (missing line number)

**Description**:
Pushes current line onto stack and branches execution to specified subroutine line number.

---

### `RETURN`

- **Category**: Control Flow
- **Syntax**: `RETURN [line_num]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 3: RETURN without GOSUB (call stack empty)

**Description**:
Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line.

---

### `IF`

- **Category**: Control Flow
- **Syntax**: `IF expr THEN stmt/line [ELSE stmt/line]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax error (missing THEN or malformed expression)

**Description**:
Evaluates boolean expression expr and executes THEN clause if true, ELSE clause if false.

---

### `END`

- **Category**: Control Flow
- **Syntax**: `END [IF | SUB | FUNCTION | SELECT | STRUCT]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax error (mismatched block terminator)

**Description**:
Terminates program execution or closes structured block definitions.

---

### `REM`

- **Category**: Control Flow
- **Syntax**: `REM [comment text] or ' [comment text]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: None (comments never generate errors)

**Description**:
Defines a remark/comment line ignored by the BASIC engine during execution.

---

### `STOP`

- **Category**: Control Flow
- **Syntax**: `STOP`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: None (STOP halts program execution cleanly)

**Description**:
Pauses program execution and prints 'Break in line X' message.

---

### `PRINT`

- **Category**: Console I/O
- **Syntax**: `PRINT [#n,] [exprlist] [;|,]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number

**Description**:
Outputs formatted text or numeric expressions to the console or open file channel.

---

### `INPUT`

- **Category**: Console I/O
- **Syntax**: `INPUT [;] ["prompt"{;|,}] var1[, var2...]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number

**Description**:
Prompts the user or reads values from console input into target variables.

---

### `SELECT`

- **Category**: Control Flow
- **Syntax**: `SELECT CASE test_expression ... CASE expression_list ... END SELECT`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 13: Type Mismatch, Error 37: SELECT Without CASE

**Description**:
Executes one of several blocks of statements depending on the value of an expression.

---

### `FOR`

- **Category**: Looping / Control Flow
- **Syntax**: `FOR var = start TO end [STEP step]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 13: Type Mismatch, Error 26: FOR Without NEXT

**Description**:
Initiates a counter-controlled loop block executing statements until var reaches end.

---

### `NEXT`

- **Category**: Looping / Control Flow
- **Syntax**: `NEXT [var1[, var2...]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 1: NEXT Without FOR, Error 2: Syntax Error

**Description**:
Increments the FOR loop counter variable and loops back if target bound has not been exceeded.

---

### `WHILE`

- **Category**: Looping / Control Flow
- **Syntax**: `WHILE condition`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 30: WHILE Without WEND

**Description**:
Executes a series of statements in a loop as long as condition evaluates to non-zero (true).

---

### `WEND`

- **Category**: Looping / Control Flow
- **Syntax**: `WEND`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 29: WEND Without WHILE, Error 2: Syntax Error

**Description**:
Terminates a WHILE...WEND loop block and transfers control back to the matching WHILE statement.

---

### `DO`

- **Category**: Looping / Control Flow
- **Syntax**: `DO [{WHILE|UNTIL} condition]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 31: DO Without LOOP

**Description**:
Initiates a structured DO...LOOP block, optionally evaluating a WHILE or UNTIL pre-condition.

---

### `LOOP`

- **Category**: Looping / Control Flow
- **Syntax**: `LOOP [{WHILE|UNTIL} condition]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 32: LOOP Without DO, Error 2: Syntax Error

**Description**:
Terminates a DO...LOOP block, optionally evaluating a post-condition test.

---

### `EXIT`

- **Category**: Looping / Control Flow
- **Syntax**: `EXIT {FOR|DO|WHILE|SUB|FUNCTION}`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 33: Invalid EXIT Scope

**Description**:
Prematurely exits an active loop, SUB, or FUNCTION block.

---

### `OPEN`

- **Category**: Filesystem I/O
- **Syntax**: `OPEN filespec [FOR mode] AS [#]file_num [LEN=reclen]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 52: Bad File Number, Error 53: File Not Found, Error 55: File Already Open

**Description**:
Opens a file channel for INPUT, OUTPUT, APPEND, BINARY, or RANDOM I/O operations.

---

### `CLOSE`

- **Category**: Filesystem I/O
- **Syntax**: `CLOSE [[#]file_num1[, [#]file_num2...]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 52: Bad File Number

**Description**:
Closes open file channels, flushing pending I/O buffers to disk.

---

### `GET`

- **Category**: Filesystem I/O
- **Syntax**: `GET [#]file_num [, record_number]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number

**Description**:
Reads a record from a random-access file into the FIELD buffer.

---

### `PUT`

- **Category**: Filesystem I/O
- **Syntax**: `PUT [#]file_num [, record_number]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number

**Description**:
Writes a record from the FIELD buffer into a random-access file.

---

### `FIELD`

- **Category**: Filesystem I/O
- **Syntax**: `FIELD [#]file_num, width AS string_var [, width AS string_var...]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 50: Field Overflow, Error 52: Bad File Number

**Description**:
Allocates space in a random file buffer for record variables.

---

### `FILES`

- **Category**: Filesystem I/O
- **Syntax**: `FILES [filespec]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied

**Description**:
Displays directory listing matching specified file pattern.

---

### `KILL`

- **Category**: Filesystem I/O
- **Syntax**: `KILL filespec`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied

**Description**:
Deletes specified file from disk storage.

---

### `MKDIR`

- **Category**: Filesystem I/O
- **Syntax**: `MKDIR pathspec`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 75: Path/File Access Error, Error 76: Path Not Found

**Description**:
Creates a new directory on disk.

---

### `NAME`

- **Category**: Filesystem I/O
- **Syntax**: `NAME oldspec AS newspec`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 53: File Not Found, Error 58: File Already Exists

**Description**:
Renames an existing disk file or directory.

---

### `SEEK`

- **Category**: Filesystem I/O
- **Syntax**: `SEEK [#]file_num, position`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number

**Description**:
Sets the byte offset position for the next read or write operation on an open file.

---

### `SCREEN`

- **Category**: Graphics & Display
- **Syntax**: `SCREEN mode [, [colorswitch] [, [active_page] [, visual_page]]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Sets text or visual graphics display mode, screen resolution, and active palette pages.

---

### `CLS`

- **Category**: Graphics & Display
- **Syntax**: `CLS [n]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Clears display: CLS -1 (text viewport), CLS -2 (graphics viewport), CLS -3 (full buffer, default), or CLS n (0-15 background color clear).

---

### `HOME`

- **Category**: Graphics & Display
- **Syntax**: `HOME [n]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Moves text cursor to top-left corner (1, 1) without clearing text screen, optionally changing text color to n (0-15).

---

### `COLOR`

- **Category**: Graphics & Display
- **Syntax**: `COLOR [foreground] [, [background] [, border]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Sets active foreground, background, and border display colors.

---

### `LINE`

- **Category**: Graphics & Display
- **Syntax**: `LINE [[x1, y1]]-(x2, y2) [, [color] [, [B|BF] [, style]]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Draws a straight line or box on the active graphics screen canvas.

---

### `CIRCLE`

- **Category**: Graphics & Display
- **Syntax**: `CIRCLE (x, y), radius [, [color] [, [start] [, [end] [, aspect]]]]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Draws a circle, ellipse, or arc on the active graphics screen canvas.

---

### `SOUND`

- **Category**: Sound & Audio
- **Syntax**: `SOUND frequency, duration`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Generates a tone of specified frequency in Hertz for specified duration in clock ticks.

---

### `PLAY`

- **Category**: Sound & Audio
- **Syntax**: `PLAY command_string`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Plays musical notes using MML (Music Macro Language) command string.

---

### `BEEP`

- **Category**: Sound & Audio
- **Syntax**: `BEEP`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Emits a standard 800 Hz speaker beep tone for 0.25 seconds.

---

### `VOICE`

- **Category**: Sound & Audio
- **Syntax**: `VOICE channel, waveform, attack, decay, sustain, release`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Configures synthesizer voice envelope parameters (ADSR) for multi-channel sound output.

---

### `ON KEY`

- **Category**: Event Trapping
- **Syntax**: `ON KEY(n) GOSUB line_label | KEY(n) {ON|OFF|STOP} | KEY ON | KEY OFF | KEY n, string`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Establishes an asynchronous interrupt handler for function key presses, or toggles/customizes row 25 function key labels.

---

### `ON TIMER`

- **Category**: Event Trapping
- **Syntax**: `ON TIMER(seconds) GOSUB line_label | TIMER {ON|OFF|STOP}`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Establishes a periodic timer interrupt subroutine trigger.

---

### `ON ERROR`

- **Category**: Event Trapping
- **Syntax**: `ON ERROR GOTO {line_label | 0}`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 5: Illegal Function Call

**Description**:
Enables error-trapping routine and specifies the first statement of the handler.

---

### `RESUME`

- **Category**: Event Trapping
- **Syntax**: `RESUME [0 | NEXT | line_label]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 20: RESUME Without Error, Error 2: Syntax Error

**Description**:
Resumes program execution after an error-handling routine finishes.

---

### `TRY`

- **Category**: Event Trapping
- **Syntax**: `TRY ... CATCH err_var ... FINALLY ... END TRY`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 35: TRY Without CATCH/FINALLY

**Description**:
Structured exception-handling block supporting TRY, CATCH, FINALLY, and END TRY.

---

### `MAT INPUT`

- **Category**: Matrix Operations
- **Syntax**: `MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 13: Type Mismatch

**Description**:
Reads numeric or string matrix elements from console input or an open file stream.

---

### `MAT PRINT`

- **Category**: Matrix Operations
- **Syntax**: `MAT PRINT [#file_num,] array_name [;|,]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 13: Type Mismatch

**Description**:
Outputs formatted 1D or 2D matrix array elements to console or file stream.

---

### `MAT READ`

- **Category**: Matrix Operations
- **Syntax**: `MAT READ array_name [(num_rows [, num_cols])]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 4: Out of DATA, Error 9: Subscript Out of Range

**Description**:
Populates matrix elements sequentially from DATA statements.

---

### `SCOPE`

- **Category**: Introspection
- **Syntax**: `SCOPE [BEGIN | END | DISABLE kw | ENABLE kw | HOOK ... | MODULE name | PRIVATE sym]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 13: Permission Denied

**Description**:
Manages lexical scope blocks, keyword enablement, execution hooks, and symbol protection.

---

### `HELP`

- **Category**: Introspection
- **Syntax**: `HELP [keyword | command | block_target]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Displays interactive help documentation for statements, functions, syntax, and system components.

---

### `REFORMAT`

- **Category**: Program Mgmt & Editing
- **Syntax**: `REFORMAT [spaces_per_indent] [target] [CHECK|STRICT|SPLIT|UPPER|LOWER|PRESERVE|SPACES]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Reformats BASIC source code with configurable indentation and static structural analysis.

---

### `CHECK`

- **Category**: Debug & Testing
- **Syntax**: `CHECK [SUMMARY|DETAILED|JSON]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Executes static analysis on active program in memory and outputs summary, detailed, or JSON metrics.

---

### `VERIFY`

- **Category**: Debug & Testing
- **Syntax**: `VERIFY ["filename.bas"] [SUMMARY|DETAILED|JSON]`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 53: File Not Found

**Description**:
Executes static analysis on external disk file or current program in isolated temporary memory.

---

### `TEST`

- **Category**: Debug & Testing
- **Syntax**: `TEST "Suite Name" | ENDTEST`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Begins or ends a declarative unit test suite block and outputs assertion metrics.

---

### `DEBUG`

- **Category**: Debug & Testing
- **Syntax**: `DEBUG [ON|OFF|DUMP|STACK|MEMORY|VARS] | TRACE | TRON | TROFF | BREAK | CONT | BACKTRACE | INFO | DUMP`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error, Error 99: Assertion Failed

**Description**:
Master interactive debugger control, execution tracing, breakpoint handling, and system introspection suite.

---

### `VERSION`

- **Category**: System & Environ
- **Syntax**: `VERSION "1.2.0" | VER | VER$([target$])`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Tags or queries active program version metadata and host engine version.

---

### `VARPTR`

- **Category**: Variables & Memory
- **Syntax**: `VARPTR(var) | VARPTR$(var)`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 2: Syntax Error

**Description**:
Returns the 64-bit integer memory address or descriptor string for a variable.

---

### `HOSTNAME`

- **Category**: Introspection
- **Syntax**: `HOSTNAME`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 70: Permission Denied

**Description**:
Outputs the system hostname to console.

---

### `USERNAME`

- **Category**: Introspection
- **Syntax**: `USERNAME`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 70: Permission Denied

**Description**:
Outputs the current user identity name to console.

---

### `SELFTEST`

- **Category**: System
- **Syntax**: `SELFTEST`
- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile
- **Error Codes**: Error 1001-5003: Subsystem Diagnostic Failure

**Description**:
Executes internal diagnostic suite verifying lexer, memory, string, variable, and array subsystems.

---

