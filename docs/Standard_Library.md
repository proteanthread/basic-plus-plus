# BASIC++ v6.5.2 Standard Library Reference

## 1. OVERVIEW

The standard library is the set of built-in functions, statements, and commands available without loading any modules. The exact set depends on the build edition: baspp (Standard) includes everything, bpp (Lite) excludes graphics, BIOS, JSON, XML, INI, and the TUI editor, and bs (Batch) excludes the REPL and all interactive features.

## 2. NUMERIC FUNCTIONS

### Basic Math
ABS(n) — Absolute value. SGN(n) — Sign: -1, 0, or 1. INT(n) — Floor (rounds toward negative infinity). FIX(n) — Truncate toward zero. CINT(n) — Round to nearest integer. CSNG(n) — Convert to single precision. CDBL(n) — Convert to double precision.

### Extended Math
SQR(n) — Square root. SIN(n) — Sine (radians). COS(n) — Cosine. TAN(n) — Tangent. ATN(n) — Arctangent. ASIN(n) — Arcsine. ACOS(n) — Arccosine. SINH(n) — Hyperbolic sine. COSH(n) — Hyperbolic cosine. TANH(n) — Hyperbolic tangent.

### Logarithmic and Exponential
LOG(n) — Natural logarithm. LOG2(n) — Log base 2. LOG10(n) — Log base 10. EXP(n) — e raised to the power n.

### Constants and Precision
PI — Returns 3.14159265358979. EPS — Machine epsilon (smallest distinguishable difference from 1.0). INF — Positive infinity. MAXNUM — Largest representable double value.

### Rounding and Clamping
ROUND(n, places) — Round to specified decimal places. FLOOR(n) — Round down. CEIL(n) — Round up. CLAMP(n, low, high) — Restrict n to the range [low, high].

### Aggregates
MIN(a, b) — Smaller of two values. MAX(a, b) — Larger of two values. AVG(list) — Arithmetic mean. MED(list) — Median value.

### Conversion and Utility
LERP(a, b, t) — Linear interpolation between a and b at position t. DEGREES(r) — Radians to degrees. RADIANS(d) — Degrees to radians. ANGLE(x, y) — Angle in radians from origin to point (x,y). ATAN2(y, x) — Two-argument arctangent. REMAINDER(a, b) — IEEE remainder. TRUNCATE(n) — Remove fractional part.

### Random Numbers
RND — Pseudo-random number between 0 and 1. RND(0) — Repeats the last random number. RND(n) where n > 0 — Next random number. RND(n) where n < 0 — Seeds the generator and returns the first value.

### Bitwise Functions
BITCOUNT(n) — Count set bits. READBIT(n, pos) — Read bit at position. SETBIT(n, pos) — Set bit at position. RESETBIT(n, pos) — Clear bit at position. TOGGLEBIT(n, pos) — Toggle bit at position. SHL(n, count) — Shift left. SHR(n, count) — Shift right.

## 3. STRING FUNCTIONS

### Core
LEN(s$) — Length. LEFT$(s$, n) — Left substring. RIGHT$(s$, n) — Right substring. MID$(s$, start, len) — Middle substring. INSTR([start,] s$, find$) — Find substring position.

### Conversion
CHR$(n) — Character from ASCII code. ASC(s$) — ASCII code from first character. STR$(n) — Number to string. VAL(s$) — String to number. HEX$(n) — Hexadecimal representation. OCT$(n) — Octal representation. BIN$(n) — Binary representation.

### Generation
SPACE$(n) — String of n spaces. STRING$(n, char) — String of n copies of a character.

### Case
UCASE$(s$) — Convert to uppercase. LCASE$(s$) — Convert to lowercase. TCASE$(s$) — Convert to title case.

### Trimming
LTRIM$(s$) — Remove leading spaces. RTRIM$(s$) — Remove trailing spaces. TRIM$(s$) — Remove both.

### Pack/Unpack
PACK$(format$, val, ...) — Pack values into binary string. UNPACK(format$, data$, var, ...) — Unpack binary string into variables.

### GW-BASIC Field Conversion
CVI(s$) — Convert 2-byte string to integer. CVS(s$) — Convert 4-byte string to single. CVD(s$) — Convert 8-byte string to double. MKI$(n) — Convert integer to 2-byte string. MKS$(n) — Convert single to 4-byte string. MKD$(n) — Convert double to 8-byte string.

## 4. SYSTEM FUNCTIONS

DATE$ — Current date as "MM-DD-YYYY". TIME$ — Current time as "HH:MM:SS". CLOCK$ — Full timestamp. TIMER — Seconds since midnight (double precision). TICKS — System tick count. FRE(0) — Free string heap space. FRE(-1) — Largest free block. FRE(-2) — Free variable space. INKEY$ — Non-blocking single character input. INPUT$(n) — Read n characters from keyboard. ENVIRON$("VAR") — Environment variable value. CSRLIN — Current cursor row. POS(0) — Current cursor column. LPOS(n) — Printer head position. ERR — Last error code. ERL — Last error line. ERR$ — Last error message. FREEFILE — Next available file channel.

## 5. FILE FUNCTIONS

EOF(n) — End of file test. LOC(n) — Current file position. LOF(n) — File length. SEEK (as function) — Current seek position.

## 6. STATEMENTS

### Program Flow
LET, GOTO, GOSUB, RETURN, IF/THEN/ELSE/ELSEIF/END IF, FOR/NEXT, WHILE/WEND, DO/LOOP, SELECT CASE/END SELECT, END, STOP, ON...GOTO, ON...GOSUB, EXIT FOR, EXIT DO, CONT.

### Variables and Data
DIM, REDIM, ERASE, OPTION BASE, DATA, READ, RESTORE, SWAP, DEFINT, DEFSNG, DEFDBL, DEFSTR, COMMON, SHARED, STATIC, LOCAL, CONST, LET.

### Procedures
SUB, END SUB, FUNCTION, END FUNCTION, CALL, DECLARE, DEF FN.

### I/O
PRINT, INPUT, LINE INPUT, WRITE, LOCATE, COLOR, CLS, HOME, WIDTH, BEEP, SOUND, PLAY, LPRINT.

### Files
OPEN, CLOSE, RESET, GET, PUT, BGET, BPUT, FIELD, LSET, RSET, SEEK, LOCK, UNLOCK, FILES, DIR, KILL, NAME, MKDIR, RMDIR, CHDIR.

### Program Management
RUN, LIST, NEW, SAVE, LOAD, MERGE, CHAIN, AUTO, RENUM, DELETE, EDIT, BSAVE, BLOAD, BRUN.

### Error Handling
ON ERROR GOTO, RESUME, ERROR, TRY, CATCH, END TRY, THROW.

### Graphics
SCREEN, PSET, PRESET, LINE, CIRCLE, PAINT, DRAW, PALETTE, POINT, VIEW, WINDOW, PCOPY, SET SCREEN, SET GRAPHICS.

### Debugging
TRON, TROFF, DEBUG, BREAK, VARS, DUMP, BACKTRACE, ASSERT, TEST, ENDTEST, VERIFY, CHECK, SELFTEST, TRACE.

### Metaprogramming
ALIAS, OVERRIDE, SCOPE, KEYWORD, REMOVE.

### System
SYSTEM, BYE, SHELL, ENVIRON, INFO, VER, HELP, CATALOG, SECURITY, RESTRICT, RANDOMIZE, CLEAR, SLEEP, PAUSE, DELAY.

### Event Trapping
ON KEY GOSUB, ON TIMER GOSUB, ON COM GOSUB, ON PEN GOSUB, ON STRIG GOSUB, ON PLAY GOSUB, ON ALARM GOSUB. Event state: ON, OFF, STOP.
