<!--
Title:        Standard Library
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/src/eval/functions/, engine/src/runtime/
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Standard Library Architecture & Builtin Reference

The authoritative reference catalog for the BASIC++ v6.5.2 standard library, its architectural design principles, and built-in intrinsic functions.

## 1. Standard Library Architecture & Philosophy

The BASIC++ v6.5.2 standard library is designed under three core principles:
1. **ISO C17 Freestanding Conformance**: Micro-libraries compile independently with zero host platform lock-in.
2. **Zero-Leak Memory Invariants**: All string and collection operations operate on deterministic lifecycle arenas or reference-counted handles.
3. **100% Backward Compatibility**: Intrinsic functions preserve vintage GW-BASIC, QuickBASIC, and ANSI Minimal BASIC behavior while offering modern high-precision scientific and text processing extensions.

---

## 2. Numeric & Mathematical Intrinsic Functions

### 2.1 Basic Arithmetic & Precision Conversion
- `ABS(n)`: Returns absolute value of `n`.
- `SGN(n)`: Returns sign (-1, 0, or 1).
- `INT(n)`: Floor function (rounds toward negative infinity).
- `FIX(n)`: Truncates fractional part toward zero.
- `CINT(n)`: Rounds to nearest 16-bit integer.
- `CLNG(n)`: Converts to 32-bit long integer.
- `CSNG(n)`: Converts to single-precision float.
- `CDBL(n)`: Converts to double-precision float.

### 2.2 Trigonometric & Extended Math
- `SIN(rad)`, `COS(rad)`, `TAN(rad)`: Standard trigonometric functions.
- `ASIN(x)`, `ACOS(x)`, `ATN(x)`: Inverse trigonometric functions.
- `SINH(x)`, `COSH(x)`, `TANH(x)`: Hyperbolic trigonometric functions.
- `ATAN2(y, x)`: Two-argument arctangent returning angle in radians.
- `SQR(n)`: Square root of non-negative number.
- `LOG(n)`: Natural logarithm (base e).
- `LOG10(n)`: Common logarithm (base 10).
- `LOG2(n)`: Binary logarithm (base 2).
- `EXP(n)`: Exponential function (e raised to power `n`).

### 2.3 Mathematical Utilities & Rounding
- `PI`: Returns constant `3.141592653589793`.
- `ROUND(n, places)`: Rounds to specified decimal precision.
- `FLOOR(n)`: Rounds down to nearest integer.
- `CEIL(n)`: Rounds up to nearest integer.
- `CLAMP(n, low, high)`: Restricts value within range `[low, high]`.
- `MIN(a, b)`, `MAX(a, b)`: Returns minimum or maximum of operands.
- `LERP(a, b, t)`: Linear interpolation between `a` and `b` at factor `t`.
- `DEGREES(rad)`: Converts radians to degrees.
- `RADIANS(deg)`: Converts degrees to radians.

### 2.4 Pseudo-Random Numbers
- `RND`: Returns uniform pseudo-random double in range `[0.0, 1.0)`.
- `RANDOMIZE seed`: Reseeds the internal linear congruential PRNG.

### 2.5 Bitwise Operations
- `BITCOUNT(n)`: Returns number of set bits in integer.
- `READBIT(n, pos)`: Returns state of bit at position `pos`.
- `SETBIT(n, pos)`, `RESETBIT(n, pos)`, `TOGGLEBIT(n, pos)`: Bit manipulation.
- `SHL(n, count)`, `SHR(n, count)`: Bitwise logical shift left and right.

---

## 3. String & Text Processing Functions

### 3.1 Substring & Slicing Operations
- `LEN(s$)`: Returns string length in bytes.
- `LEFT$(s$, n)`: Returns leftmost `n` characters.
- `RIGHT$(s$, n)`: Returns rightmost `n` characters.
- `MID$(s$, start [, len])`: Returns middle substring.
- `INSTR([start,] s$, find$)`: Searches for substring from left.
- `RINSTR([start,] s$, find$)`: Searches for substring from right.

### 3.2 String Conversions & Formatting
- `CHR$(n)`: Returns character for ASCII/byte code `n`.
- `ASC(s$)`: Returns ASCII code of first character.
- `STR$(n)`: Converts numeric value to formatted string.
- `VAL(s$)`: Parses numeric value from string.
- `HEX$(n)`, `OCT$(n)`, `BIN$(n)`: Converts integer to hexadecimal, octal, or binary string.
- `SPACE$(n)`: Generates string of `n` space characters.
- `STRING$(n, char$)`: Generates string of `n` repeating characters.

### 3.3 Case & Whitespace Transformations
- `UCASE$(s$)`: Converts text to uppercase.
- `LCASE$(s$)`: Converts text to lowercase.
- `TCASE$(s$)`: Converts text to title case.
- `LTRIM$(s$)`, `RTRIM$(s$)`, `TRIM$(s$)`: Trims leading and trailing whitespace.

### 3.4 Binary Packing and Field Buffers
- `PACK$(format$, val, ...)`: Serializes values into binary struct string.
- `UNPACK(format$, data$, var, ...)`: Deserializes binary data into variables.
- `CVI`, `CVS`, `CVD`: Converts 2-, 4-, or 8-byte fielded strings to numeric values.
- `MKI$`, `MKS$`, `MKD$`: Converts numeric values to fielded string records.

---

## 4. System & Environmental Functions

- `ENVIRON$(key$)`: Queries host environment variable value.
- `COMMAND$`: Returns command-line argument string.
- `DATE$`: Returns current calendar date (`YYYY-MM-DD`).
- `TIME$`: Returns current time of day (`HH:MM:SS`).
- `TIMER`: Returns elapsed seconds since midnight with microsecond precision.
- `FRE(0)`: Returns available memory in the active program pool.
- `ERR`: Returns the active runtime error code.
- `ERL`: Returns the line number where the last error occurred.
- `PEEK(addr)`: Reads raw byte from virtual memory address.
- `INP(port)`: Reads byte from virtual hardware I/O port.
