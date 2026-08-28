# BASIC++ v6.5.2 Super BASIC Compatibility & Extended Capabilities

## 1. History & Architectural Overview

SDS 940 (1968) and DEC PDP-10 (1970–1972) Super BASIC (pioneered by Tymshare Inc.) introduced groundbreaking language features: native complex numbers, bidirectional string/ASCII array conversions (`CHANGE`), fine-grained console and file layout control (`MARGIN`, `ZONE`), direct text and modify file modes (`TEXT`, `APPEND`, `MODIFY`), template-based formatted output (`IMAGE`, `FORM`), line input preservation (`LINPUT`), and full matrix file I/O (`MAT READ`, `MAT PRINT #`, `MAT INPUT #`).

In BASIC++ v6.5.2, all of these features are implemented as first-class native engine capabilities with 100% backward compatibility for GW-BASIC, BASICA, and QBASIC.

---

## 2. Native Complex Number System (Phase 1)

### Imaginary Literals & Construction
Numbers with trailing `I` or `i` are lexed directly as imaginary constants:
```basic
10 LET Z1 = 3 + 4I
20 LET Z2 = CMPLX(5, -12)
30 PRINT "Z1 = "; CMPLX$(Z1); " | Z2 = "; CMPLX$(Z2)
```

### Complex Arithmetic & Relational Operators
All standard arithmetic operations are fully supported on complex numbers:
```basic
10 LET Z_ADD = (3 + 4I) + (1 - 2I)      ' Result: 4 + 2I
20 LET Z_SUB = (3 + 4I) - (1 + 2I)      ' Result: 2 + 2I
30 LET Z_MUL = (3 + 4I) * (1 - 2I)      ' Result: 11 - 2I
40 LET Z_DIV = (11 - 2I) / (1 - 2I)     ' Result: 3 + 4I
50 LET Z_NEG = -(3 + 4I)                ' Result: -3 - 4I
60 LET Z_POW = (0 + 1I) ^ 2             ' Result: -1 + 0I
70 IF Z1 == 3 + 4I THEN PRINT "Equal"
```

### Complex Built-in Functions
| Function | Description |
|----------|-------------|
| `CMPLX(r, i)` | Constructs a complex number from real and imaginary parts |
| `REAL(z)` | Extracts the real component |
| `IMAG(z)` | Extracts the imaginary component |
| `CONJ(z)` | Returns the complex conjugate ($a - bi$) |
| `ARG(z)` | Returns the phase angle in radians ($\text{atan2}(b, a)$) |
| `CMPLX$(z)` | Converts complex number to formatted string (`3+4I`) |
| `CSQR(z)` | Complex principal square root |
| `CLOG(z)` | Complex natural logarithm ($\ln |z| + i \arg(z)$) |
| `CEXP(z)` | Complex exponential ($e^z$) |
| `CSIN(z)` | Complex sine |
| `CCOS(z)` | Complex cosine |
| `CTAN(z)` | Complex tangent |
| `CASN(z)` | Complex inverse sine |
| `CACS(z)` | Complex inverse cosine |
| `CATN(z)` | Complex inverse tangent |

### Type Declarations & Binary Serialization
```basic
10 DEFCPX Z                          ' Variables starting with Z default to Complex
20 COMPLEX W1, W2, W_ARR(10)         ' Explicit complex variable & array declaration
30 LET S$ = MKC$(3 + 4I)             ' 16-byte IEEE Little-Endian serialization
40 LET Z = CVC(S$)                   ' Deserialization
50 LET L$ = MKL$(100000&)            ' 4-byte IEEE 32-bit integer serialization
60 LET N& = CVL(L$)
```

---

## 3. String & ASCII Conversion (`CHANGE`)

The `CHANGE` statement converts bidirectionally between strings and 1D numeric ASCII arrays. `A(0)` contains the string length:

```basic
10 LET S$ = "HELLO"
20 CHANGE S$ TO A                    ' A(0)=5, A(1)=72, A(2)=69, A(3)=76, A(4)=76, A(5)=79
30 LET A(1) = 74                     ' Replace 'H' with 'J'
40 CHANGE A TO RES$                  ' RES$ = "JELLO"
50 LET A(0) = 3                      ' Truncate length
60 CHANGE A TO SHORT$                ' SHORT$ = "JEL"
```

---

## 4. Console & File Output Formatting (`MARGIN`, `ZONE`, `IMAGE`, `FORM`)

### Line Margins and Comma Print Zones
```basic
10 ZONE 10                           ' Set comma tab width to 10 columns (default: 14)
20 MARGIN 132                        ' Set line wrap margin to 132 columns (default: 80)
30 PRINT "Col1", "Col2", "Col3"
40 ZONE #1, 20                       ' Set zone width specifically for file channel #1
50 MARGIN #1, 120
```

### Formatted Templates (`IMAGE` & `FORM`)
```basic
100 IMAGE : Total: $$$$,$$$.##   Status: \      \
110 PRINT USING 100, 1250.75, "APPROVED"
120 FORM C10, N5.2, C5
```

---

## 5. Timesharing File I/O & Matrix I/O

### Dedicated File Open Statements
- `TEXT [#]ch, "file"`: Opens a sequential file in text output mode (equivalent to `OPEN file FOR OUTPUT`).
- `APPEND [#]ch, "file"`: Opens a sequential file in append mode (equivalent to `OPEN file FOR APPEND`).
- `MODIFY [#]ch, "file"`: Opens a file for read/write random access modify mode.
- `LINPUT [#ch,] ["prompt";] var$`: Reads an entire line of text preserving commas, quotes, and whitespace.

### Matrix File I/O & `MAT READ`
```basic
10 OPTION BASE 1
20 DATA 10, 20, 30, 40, 50, 60
30 DIM M(2, 3)
40 MAT READ M                        ' Populate 2x3 matrix from DATA items
50 TEXT #1, "matrix.dat"
60 MAT PRINT #1, M                   ' Write formatted matrix to file
70 CLOSE #1
80 OPEN "matrix.dat" FOR INPUT AS #1
90 DIM M2(2, 3)
100 MAT INPUT #1, M2                 ' Read matrix from file
110 CLOSE #1
120 MAT PRINT M2                     ' Display matrix on console
```

---

## 6. Advanced Math, Trigonometric, Hyperbolic & Matrix Operations

### Hyperbolic Functions
- `SINH(x)` / `HSN(x)`: Hyperbolic sine ($\sinh(x) = \frac{e^x - e^{-x}}{2}$)
- `COSH(x)` / `HCS(x)`: Hyperbolic cosine ($\cosh(x) = \frac{e^x + e^{-x}}{2}$)
- `TANH(x)` / `HTN(x)`: Hyperbolic tangent ($\tanh(x) = \frac{\sinh(x)}{\cosh(x)}$)

### Reciprocal Trigonometric & Inverse Aliases
- `COT(x)`: Trigonometric cotangent ($1 / \tan(x)$)
- `SEC(x)`: Trigonometric secant ($1 / \cos(x)$)
- `CSC(x)`: Trigonometric cosecant ($1 / \sin(x)$)
- `ASN(x)` / `ASIN(x)`: Inverse arcsine (in radians, $-1.0 \le x \le 1.0$)
- `ACS(x)` / `ACOS(x)`: Inverse arccosine (in radians, $-1.0 \le x \le 1.0$)

### Angular Conversions, Logarithms & Utilities
- `RAD(deg)`: Converts degrees to radians ($\text{deg} \times \frac{\pi}{180}$)
- `DEG(rad)`: Converts radians to degrees ($\text{rad} \times \frac{180}{\pi}$)
- `LOG10(x)` / `LGT(x)`: Base-10 common logarithm
- `LOG2(x)`: Base-2 binary logarithm
- `EXN(x)` / `EXP(x)`: Natural exponential function ($e^x$)
- `COMP(a, b)`: Numeric comparison returning `-1` ($a < b$), `0` ($a = b$), or `1` ($a > b$)
- `PDIF(a, b)`: Positive difference ($a - b$ if $a > b$, else `0`)
- `DET` / `DET()` / `DET(A)`: Matrix determinant (read last calculated determinant or evaluate square matrix `A`)

### Matrix Algebraic Operations
```basic
10 OPTION BASE 1
20 DIM A(2, 2), B(2, 2), V(2), M(2, 2)
30 ' Matrix scalar arithmetic:
40 MAT B = A + 5                    ' Scalar addition
50 MAT B = A - 2                    ' Scalar subtraction
60 MAT B = 3 * A                    ' Scalar multiplication (or MAT B = A * 3)
70 MAT B = A / 10                   ' Scalar division
80 ' Mixed Vector-Matrix multiplication:
90 MAT RES1 = V * M                 ' 1D row vector (1xN) * 2D matrix (NxP) -> 1D vector (1xP)
100 MAT RES2 = M * V                ' 2D matrix (MxN) * 1D column vector (Nx1) -> 1D vector (Mx1)
110 ' String Matrix Operations:
120 DIM S$(2, 2), S_COPY$(2, 2)
130 MAT S$ = NUL$                   ' Clear string matrix to empty strings
140 MAT S_COPY$ = S$                ' Copy string matrix with reference counting
```

---

## 7. Universal Timesharing & DEC PDP-10 / Tymshare Integration (Phase 6)

### Syntax & Directives
- `EXTEND` / `NO EXTEND`: Enables or disables extended multi-character identifiers and spaces within variable names.
- `SCALE factor`: Sets fixed-point decimal scaling and arithmetic rounding (DEC PDP-10 / BASIC-PLUS).
- `\` (Backslash): Multi-statement line separator across all timesharing dialects:
  ```basic
  10 LET A = 10 \ LET B = 20 \ PRINT A + B
  ```
- `&` (Ampersand): Infix string concatenation operator with automatic numeric coercion:
  ```basic
  10 LET S$ = "SCORE: " & 100 & " POINTS"
  ```
- Substring Slice Assignment: Mutate string segments in-place:
  ```basic
  10 LET A$ = "HELLO WORLD"
  20 LET A$[1, 5] = "HOWDY"           ' A$ = "HOWDY WORLD"
  30 LET A$(7 TO 11) = "THERE"        ' A$ = "HOWDY THERE"
  ```
- Print Spacing Modifiers: `LIN(n)` (print $n$ blank lines) and `SPA(n)` (print $n$ spaces).

### Terminal Control & Timed Execution
- `ECHO` / `NO ECHO`: Enable or disable console character echoing for password / masked input.
- `ENTER timeout_secs, var`: Timed input that aborts after specified seconds if no input is received.
- `SLEEP [seconds]`: Non-blocking / timer-based suspension.
- `PAUSE [message$]`: Interactive pause awaiting user keypress.
- `WAIT`: Disambiguated tri-mode waiting:
  - `WAIT port, and_mask [, xor_mask]`: Port polling.
  - `WAIT #channel, seconds`: Channel I/O timeout limit.
  - `WAIT seconds`: Time delay.

### Storage, Channel Control & Program Overlay
- `FILES`: General directory listing with pattern matching (`FILES "*.BAS"`).
- `DIR`: Directory listing strictly filtered to `.BAS` / `.bas` files.
- `PWD`: Print current working directory path.
- `PATH ["dir"]`: Display or set executable / library search paths.
- `UNSAVE "file"` / `SCRATCH "file"`: Delete files from storage or clear channel buffers.
- `APPEND "file.bas"`: Overlay program file from disk into memory, replacing duplicated lines.
- `ASSIGN "file" TO #ch`: Bind file path or device alias to channel number.
- `ADVANCE #ch, n`: Advance channel file pointer forward by $n$ bytes.
- `RECORD #ch, rec_num`: Direct access seek to record number on random file channel.
- `OPEN "file" AS FILE [#]ch [RECORDSIZE n]`: Timesharing `AS FILE` channel declaration.
- `IF END #ch THEN line`: Channel end-of-file condition branch.
- `RESTORE [#ch]`: Rewind DATA pointer or reset file channel read pointer to beginning.

### Data Conversion & System Functions
| Function | Description |
|----------|-------------|
| `CVT$%(n%)` | Converts 16-bit integer to 2-byte PDP-10 ASCII string |
| `CVT%$(s$)` | Converts 2-byte ASCII string to 16-bit integer |
| `CVT$F(f!)` | Converts single-precision float to 4-byte IEEE string |
| `CVTF$(s$)` | Converts 4-byte IEEE string to single-precision float |
| `SWAP%(n%)` | Swaps high and low bytes of 16-bit integer |
| `RAD$(n&)` | Converts 32-bit integer to 6-character Radix-50 ASCII string |
| `NUM$(n)` | Formats number to string without leading positive space |
| `NUM1$(n)` | Formats number to compact unformatted string |
| `VAL%(s$)` | Parses integer value from numeric string (DEC PDP-10) |
| `XLATE$(src$, table$)` | Translates characters in `src$` through lookup `table$` |
| `MAG(x, y)` / `MAG(z)` | Computes vector Euclidean length ($\sqrt{x^2 + y^2}$) or complex modulus ($|z|$) |
| `SYS(code)` | Dispatches operating system / monitor services (0=exit, 1=pid, 2=uptime, 3=ram) |
| `TIME(code)` | Dispatches system timing metrics (0=seconds since midnight, 1=cpu ticks, 2=elapsed mins, 3=timer) |
