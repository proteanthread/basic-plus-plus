# SUPER BASIC (Tymshare SDS-940) Dialect Reference

**Version 4.2.3**

---

## Table of Contents

- History
- Activation
- Extended Math Functions
  - Inverse Trigonometric
  - Hyperbolic
  - Logarithms
  - Comparison & Utility
  - Constant
- UNLESS Statement (JOSS-Style Negated IF)
- BY Keyword (FOR Loop Step Alternative)
- SCRATCH and UNSAVE Commands
  - SCRATCH
  - UNSAVE
- Complex Numbers
- Dialect Configuration
- Complete Function Reference
- See Also

---

**Dialect Code:** `SUPB`

---

## History

SUPER BASIC was developed by Mark Alexander and Dan Streng at Tymshare, Inc. for the Scientific Data Systems (SDS) 940 mainframe computer, circa 1968.

Tymshare was a pioneering time-sharing company that allowed multiple users to access a central mainframe via teletype terminals connected over telephone lines. At its peak, Tymshare served thousands of simultaneous users across the United States.

SUPER BASIC was one of the most advanced BASIC dialects of its era. While microcomputer BASICs were still years away, SUPER BASIC already offered features that wouldn't appear on personal computers until the 1980s:

- Complex number arithmetic (real + imaginary)
- Double precision via `#` suffix
- Extended mathematical functions (inverse trig, hyperbolics)
- Comparison and utility functions (`COMP`, `PDIF`)
- `PI` as a built-in constant
- JOSS-inspired postfix modifiers (`UNLESS`)
- `FOR...BY...TO` ordering (step before limit)
- Enhanced matrix operations

The SDS 940 was a 24-bit computer with hardware paging and memory protection — advanced for 1968. It ran the Berkeley Timesharing System (BTS), which supported up to 64 simultaneous users.

---

## Activation

```basic
DIALECT "SUPB"
```

This changes:
- Statement separator to `:` (colon)
- `LET` is mandatory (like ECMA-55)
- Maximum line number extends to 99999
- Ready prompt: `READY`
- `CLS` is not available (mainframes used teletype output)

---

## Extended Math Functions

### Inverse Trigonometric

| Function | Description | Domain | Range |
|----------|-------------|--------|-------|
| `ASIN(x)` | Arcsine (radians) | −1 ≤ x ≤ 1 | −π/2 to π/2 |
| `ACOS(x)` | Arccosine (radians) | −1 ≤ x ≤ 1 | 0 to π |

```basic
PRINT ASIN(1)       ' 1.5708 (PI/2)
PRINT ASIN(0.5)     ' 0.5236 (PI/6)
PRINT ACOS(0)       ' 1.5708 (PI/2)
PRINT ACOS(-1)      ' 3.14159 (PI)
```

### Hyperbolic

| Function | Description | Formula |
|----------|-------------|---------|
| `SINH(x)` | Hyperbolic sine | (e^x − e^(−x)) / 2 |
| `COSH(x)` | Hyperbolic cosine | (e^x + e^(−x)) / 2 |
| `TANH(x)` | Hyperbolic tangent | SINH(x) / COSH(x) |

```basic
PRINT SINH(0)       ' 0
PRINT SINH(1)       ' 1.1752
PRINT COSH(0)       ' 1
PRINT COSH(1)       ' 1.5431
PRINT TANH(0)       ' 0
PRINT TANH(1)       ' 0.7616
```

Identity: `COSH(x)^2 - SINH(x)^2 = 1`

### Logarithms

| Function | Description | Formula |
|----------|-------------|---------|
| `LOG10(x)` | Common logarithm (base 10) | LOG(x) / LOG(10) |
| `LOG2(x)` | Binary logarithm (base 2) | LOG(x) / LOG(2) |

```basic
PRINT LOG10(100)    ' 2
PRINT LOG10(1000)   ' 3
PRINT LOG2(8)       ' 3
PRINT LOG2(256)     ' 8
PRINT LOG2(1024)    ' 10
```

### Comparison & Utility

| Function | Description |
|----------|-------------|
| `COMP(a, b)` | Three-way comparison: −1 if a < b, 0 if a = b, +1 if a > b |
| `PDIF(a, b)` | Positive difference: MAX(a − b, 0) |

```basic
PRINT COMP(3, 5)    ' -1
PRINT COMP(5, 5)    ' 0
PRINT COMP(7, 3)    ' 1

PRINT PDIF(10, 3)   ' 7
PRINT PDIF(3, 10)   ' 0
```

### Constant

`PI` — Mathematical constant π = 3.14159265358979...

```basic
PRINT PI            ' 3.14159265358979
LET RAD45 = PI / 4
PRINT SIN(PI / 2)   ' 1
```

> In standard BASIC, you must compute PI as `LET PI = 4 * ATN(1)`. SUPER BASIC provides it as a built-in constant.

---

## UNLESS Statement (JOSS-Style Negated IF)

SUPER BASIC borrowed the `UNLESS` keyword from JOSS (1963), the Johnniac Open Shop System developed at RAND Corporation.

`UNLESS` is the negated form of `IF`. It executes its `THEN` clause only when the condition is **false**. Equivalent to `IF NOT (condition) THEN statement`.

```basic
10 LET X = 5
20 UNLESS X = 0 THEN PRINT "X is nonzero"
30 UNLESS X > 10 THEN PRINT "X is not greater than 10"
40 UNLESS X < 0 THEN PRINT "X is not negative"
```

Output:
```
X is nonzero
X is not greater than 10
X is not negative
```

**Block form** (with `END IF`):

```basic
10 UNLESS X = 0
20   PRINT "X is nonzero"
30   PRINT "Its value is"; X
40 END IF
```

`UNLESS` is particularly useful for guard clauses:

```basic
100 UNLESS LEN(A$) > 0 THEN GOTO 200
110 REM ... process non-empty string ...
200 REM ... continue ...
```

---

## BY Keyword (FOR Loop Step Alternative)

In SUPER BASIC, the step size in a `FOR` loop was specified using `BY` instead of `STEP`:

```basic
FOR I = 1 TO 10 BY 2
```

Equivalent to:

```basic
FOR I = 1 TO 10 STEP 2
```

Both `BY` and `STEP` are accepted in BASIC++. The original Tymshare SUPER BASIC also supported reversed ordering (`BY` before `TO`):

```basic
FOR I = 1 BY 2 TO 10
```

**Examples:**

```basic
10 REM Count by twos
20 FOR I = 1 TO 10 BY 2
30   PRINT I;
40 NEXT I
50 PRINT
' Output: 1 3 5 7 9

10 REM Countdown
20 FOR I = 10 TO 1 BY -1
30   PRINT I;
40 NEXT I
50 PRINT
' Output: 10 9 8 7 6 5 4 3 2 1
```

---

## SCRATCH and UNSAVE Commands

### SCRATCH

```basic
SCRATCH "temp.dat"
```

Deletes a file from disk. This is the SUPER BASIC equivalent of `KILL` in Microsoft BASICs. If the file does not exist, a "File not found" message is displayed (no error is raised).

### UNSAVE

```basic
UNSAVE
```

Deletes the currently-loaded program's save file from disk. In the original Tymshare system, this removed the program from the user's file space on the mainframe.

---

## Complex Numbers

SUPER BASIC was one of the first BASIC dialects to natively support complex number arithmetic. In BASIC++, complex numbers use the `COMPLEX()` function:

```basic
10 LET Z = COMPLEX(3, 4)    ' 3 + 4i
20 PRINT REAL(Z)              ' 3
30 PRINT IMAG(Z)              ' 4
```

Complex numbers support standard arithmetic (`+`, `-`, `*`, `/`) and can be used with `ABS()` to compute the modulus (magnitude).

---

## Dialect Configuration

| Property | Value |
|----------|-------|
| Dialect ID | `DIALECT_SBASIC` |
| Short code | `"SUPB"` |
| Dialect flag | `DFLAG_SBAS` (bit 15) |
| Separator | `:` (colon) |
| LET required | Yes |
| THEN required | Yes |
| Max line number | 99999 |
| Ready prompt | `"READY"` |
| Print zone width | 15 |
| CLS available | No (teletype output) |
| Floating point | Yes |
| Strings | Yes |
| Arrays | Yes |
| FOR/NEXT | Yes |
| WHILE/WEND | No |
| DO/LOOP | No |
| DATA/READ | Yes |
| DEF FN | Yes |
| Extended variables | Yes |
| ON ERROR | No |
| TRON/TROFF | Yes |
| MERGE/CHAIN | No |

---

## Complete Function Reference

| Function | Args | Description |
|----------|------|-------------|
| `ASIN(x)` | 1 | Arcsine (radians) |
| `ACOS(x)` | 1 | Arccosine (radians) |
| `SINH(x)` | 1 | Hyperbolic sine |
| `COSH(x)` | 1 | Hyperbolic cosine |
| `TANH(x)` | 1 | Hyperbolic tangent |
| `LOG10(x)` | 1 | Common logarithm (base 10) |
| `LOG2(x)` | 1 | Binary logarithm (base 2) |
| `COMP(a, b)` | 2 | Compare (−1, 0, +1) |
| `PDIF(a, b)` | 2 | Positive difference MAX(a−b, 0) |
| `PI` | 0 | Constant 3.14159265... |

Plus all standard BASIC functions: `ABS`, `ATN`, `COS`, `EXP`, `FIX`, `INT`, `LOG`, `RND`, `SGN`, `SIN`, `SQR`, `TAN`, and string functions.

---

## See Also

- [Quick_Reference](Quick_Reference.md) — Complete keyword listing
- [Specific_Machine_Dialects](Specific_Machine_Dialects.md) — All 16 dialects explained
- [Mixing_Dialects](Mixing_Dialects.md) — Using features from multiple dialects

*@COPYLEFT ALL WRONGS RESERVED*
