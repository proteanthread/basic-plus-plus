# BASIC++ Quick Reference Card

**Version 2.0.0**

Alphabetical listing of every keyword, statement, function, and command in BASIC++.

---

## Commands (Direct Mode)

| Command | Description |
|---------|-------------|
| `ALIAS "name" = KEYWORD` | Define keyword alias |
| `ALIAS LIST` | List active aliases |
| `ALIAS CLEAR "name"` | Remove alias |
| `ALIAS CLEAR ALL` | Remove all aliases |
| `BYE` | Exit interpreter to OS prompt |
| `CONT` | Continue after STOP |
| `DELETE n1-n2` | Delete line range |
| `DIALECT "code"` | Switch BASIC dialect |
| `FILES [pattern]` | List directory |
| `HELP [keyword]` | Show help |
| `LIST [n1[-n2]]` | List program lines |
| `LOAD "filename"` | Load program |
| `MEMMAP "name"` | Select memory map |
| `MERGE "filename"` | Merge file into program |
| `NEW` | Clear program and variables |
| `RENUM [start [, step]]` | Renumber lines |
| `RUN [line]` | Execute program |
| `SAVE ["filename"]` | Save program (auto-names if bare) |
| `SELFTEST` | Run built-in tests |
| `SYSTEM` | Exit BASIC++ |
| `TROFF` | Disable trace mode |
| `TRON` | Enable trace mode |

---

## Statements

| Statement | Description |
|-----------|-------------|
| `BEEP` | Audible beep |
| `BEEP ON / OFF` | Toggle error beep |
| `CALL name(args)` | Call a SUB procedure |
| `CHAIN "filename"` | Load + run, keep variables |
| `CHDIR "path"` | Change directory |
| `CIRCLE (x,y), r [,c]` | Draw circle |
| `CLEAR` | Clear all variables |
| `CLOSE [#n, ...]` | Close file channel(s) |
| `CLS` | Clear screen |
| `COLOR fg [, bg]` | Set text color |
| `COMMON [SHARED] var, ...` | Share vars with CHAIN |
| `CONST name = value` | Define constant |
| `DATA value, value, ...` | Inline data |
| `DEF FN name(p) = expr` | Define function |
| `DEF SEG [= segment]` | Set memory segment |
| `DEF USR [= address]` | Define USR address |
| `DEFDBL letter[-letter]` | Default double precision |
| `DEFINT letter[-letter]` | Default integer |
| `DEFSNG letter[-letter]` | Default single precision |
| `DEFSTR letter[-letter]` | Default string |
| `DIM var(s1 [, s2 [, s3]])` | Declare array (1D/2D/3D) |
| `DO [WHILE\|UNTIL cond]` | Begin DO loop |
| `DRAW "commands"` | Turtle graphics |
| `END` | Terminate program |
| `ENVIRON "var=value"` | Set environment variable |
| `ERASE var [, var, ...]` | Delete array |
| `ERROR n` | Raise error |
| `EXEC "command"` | Fire-and-forget command |
| `FIELD #n, w AS v$ [, ...]` | Map record fields |
| `FOR v = s TO e [STEP i]` | Begin FOR loop (`BY` = STEP alias) |
| `FUNCTION name(params)` | Begin user function |
| `GET #n [, record]` | Read record/bytes |
| `GOSUB line` | Call subroutine |
| `GOTO line` | Unconditional jump |
| `IF cond THEN ... [ELSE ...]` | Conditional |
| `INPUT [prompt;] var` | Read keyboard |
| `INPUT #n, var` | Read from file |
| `IOCTL #n, string$` | Send device control |
| `KEY n, "string"` | Define function key |
| `KEY ON / OFF / LIST` | Function key display |
| `KILL "filename"` | Delete file |
| `LET var = expr` | Assignment (LET optional) |
| `LINE (x1,y1)-(x2,y2) [,c]` | Draw line/box |
| `LINE INPUT [prompt;] var$` | Read full line |
| `LINE INPUT #n, var$` | Read line from file |
| `LOCATE row, col` | Move cursor |
| `LOCK #n [, s TO e]` | Lock file region |
| `LOOP [WHILE\|UNTIL cond]` | End DO loop |
| `LPRINT expr` | Print to error device |
| `LSET var$ = expr$` | Left-justify in field |
| `MID$(v$, start, len) = s$` | Replace substring |
| `MKDIR "dirname"` | Create directory |
| `NAME "old" AS "new"` | Rename file |
| `NEXT [var]` | End FOR loop |
| `ON ERROR GOTO line` | Set error handler |
| `ON expr GOSUB l1, l2, ...` | Computed GOSUB |
| `ON expr GOTO l1, l2, ...` | Computed GOTO |
| `ON KEY(n) GOSUB line` | Key event handler |
| `ON TIMER(n) GOSUB line` | Timer event handler |
| `OPEN file FOR mode AS #n` | Open file channel |
| `OPTION BASE 0\|1` | Array base index |
| `OUT port, value` | Write to I/O port |
| `PAINT (x,y) [, fc [, bc]]` | Flood fill |
| `PALETTE [attr, color]` | Set color palette |
| `PCOPY src, dst` | Copy video page |
| `PLAY "mml_string"` | Play music |
| `POKE addr, value` | Write byte to memory |
| `PRESET (x, y)` | Clear pixel |
| `PRINT expr [; expr] [, ...]` | Output to screen |
| `PRINT #n, expr` | Output to file |
| `PRINT USING fmt; expr` | Formatted output |
| `PSET (x, y) [, color]` | Plot pixel |
| `PUT #n [, record]` | Write record/bytes |
| `RANDOMIZE [seed]` | Seed random generator |
| `READ var [, var, ...]` | Read from DATA |
| `REM comment` | Comment (`'` also works) |
| `RESET` | Close all files |
| `RESTORE [line]` | Reset DATA pointer |
| `RESUME [NEXT \| line]` | Resume after error |
| `RETURN` | Return from GOSUB |
| `RMDIR "dirname"` | Remove directory |
| `RSET var$ = expr$` | Right-justify in field |
| `SCRATCH "filename"` | Delete file (SBASIC alias) |
| `SCREEN mode` | Set graphics mode |
| `SECURITY LEVEL n` | Set security level |
| `SEEK #n, position` | Set file position |
| `SHELL ["command"]` | Execute OS command |
| `SLEEP [seconds]` | Pause execution |
| `SOUND freq, duration` | Generate tone |
| `STOP` | Break into direct mode |
| `SUB name(params)` | Begin subroutine |
| `SWAP var1, var2` | Exchange values (scalars/arrays) |
| `UNLESS cond THEN stmt` | Negated IF (SBASIC) |
| `UNLOCK #n [, s TO e]` | Unlock file region |
| `UNSAVE` | Delete last saved file |
| `VIEW [[SCREEN] (x1,y1)-(x2,y2)]` | Set viewport |
| `VIEW PRINT [top TO bottom]` | Set text viewport |
| `WAIT port, AND [, XOR]` | Wait for I/O condition |
| `WEND` | End WHILE loop |
| `WHILE condition` | Begin WHILE loop |
| `WIDTH columns` | Set screen width |
| `WINDOW [(x1,y1)-(x2,y2)]` | Set coordinate system |
| `WRITE [expr, ...]` | Quoted/delimited output |
| `WRITE #n, [expr, ...]` | Write to file |

---

## Numeric Functions

| Function | Description |
|----------|-------------|
| `ABS(x)` | Absolute value |
| `ACOS(x)` | Arccosine (SBASIC) |
| `ASIN(x)` | Arcsine (SBASIC) |
| `ASC(s$)` | ASCII code of first char |
| `ATN(x)` | Arctangent (radians) |
| `AVG(a, b, ...)` | Average (mean) |
| `CDBL(x)` | Convert to double |
| `CINT(x)` | Round to integer |
| `COMP(a, b)` | Compare −1/0/+1 (SBASIC) |
| `COMPLEX(re, im)` | Create complex number |
| `COS(x)` | Cosine |
| `COSH(x)` | Hyperbolic cosine (SBASIC) |
| `CSNG(x)` | Convert to single |
| `CSRLIN` | Current cursor row |
| `CVI(s$)` | 2-byte string to integer |
| `CVS(s$)` | 4-byte string to single |
| `CVD(s$)` | 8-byte string to double |
| `EOF(n)` | End of file (−1 or 0) |
| `ERL` | Error line number |
| `ERR` | Error code |
| `ERRORLEVEL` | Last SHELL exit code |
| `EXP(x)` | e^x |
| `FIX(x)` | Truncate toward zero |
| `FRE(x)` | Free memory |
| `IMAG(z)` | Imaginary part of complex |
| `INP(port)` | Read I/O port |
| `INSTR(h$, n$)` | Find substring |
| `INT(x)` | Floor |
| `LBOUND(a, dim)` | Array lower bound |
| `LEN(s$)` | String length |
| `LOC(n)` | File position |
| `LOF(n)` | File length |
| `LOG(x)` | Natural log |
| `LOG10(x)` | Common log base 10 (SBASIC) |
| `LOG2(x)` | Binary log base 2 (SBASIC) |
| `LPOS(x)` | Printer position |
| `MAX(a, b, ...)` | Maximum value |
| `MED(a, b, ...)` | Median value |
| `MIN(a, b, ...)` | Minimum value |
| `PDIF(a, b)` | Positive difference (SBASIC) |
| `PEEK(addr)` | Read memory byte |
| `PI` | Constant 3.14159... (SBASIC) |
| `POINT(x, y)` | Pixel color |
| `POS(x)` | Cursor column |
| `REAL(z)` | Real part of complex |
| `RND(x)` | Random number (0–1) |
| `ROUND(x [, n])` | Round to N decimal places |
| `SCREEN(r, c)` | Character at position |
| `SGN(x)` | Sign (−1, 0, 1) |
| `SIN(x)` | Sine |
| `SINH(x)` | Hyperbolic sine (SBASIC) |
| `SQR(x)` | Square root |
| `TAN(x)` | Tangent |
| `TANH(x)` | Hyperbolic tangent (SBASIC) |
| `TIMER` | Seconds since midnight |
| `UBOUND(a, dim)` | Array upper bound |
| `USR(x)` | User-defined machine fn |
| `VAL(s$)` | String to number |

---

## String Functions

| Function | Description |
|----------|-------------|
| `CHR$(n)` | ASCII code to character |
| `COMMAND$` | Command line arguments |
| `DATE$` | Current date |
| `ENVIRON$(name$)` | Environment variable |
| `HEX$(n)` | Hex representation |
| `INKEY$` | Non-blocking key read |
| `INPUT$(n [, #chan])` | Read n characters |
| `IOCTL$(#n)` | Device status string |
| `LCASE$(s$)` | Convert to lowercase |
| `LEFT$(s$, n)` | First n characters |
| `LTRIM$(s$)` | Remove leading spaces |
| `MID$(s$, start [, n])` | Substring |
| `MKD$(n)` | Double to 8-byte string |
| `MKI$(n)` | Integer to 2-byte string |
| `MKS$(n)` | Single to 4-byte string |
| `OCT$(n)` | Octal representation |
| `RIGHT$(s$, n)` | Last n characters |
| `RTRIM$(s$)` | Remove trailing spaces |
| `SHELL$(cmd$)` | Capture command output |
| `SPACE$(n)` | String of n spaces |
| `SPC(n)` | Print n spaces |
| `STR$(n)` | Number to string |
| `STRING$(n, ch)` | Repeated character |
| `TAB(n)` | Move to column n |
| `TIME$` | Current time |
| `UCASE$(s$)` | Convert to uppercase |
| `VARPTR$(var)` | Variable address string |

---

## Operators (Precedence Order)

| Operator | Description |
|----------|-------------|
| `^` | Exponentiation |
| `- (unary)`, `NOT` | Negation |
| `*`, `/`, `\` | Multiply, divide, int-divide |
| `MOD` | Modulo |
| `+`, `-` | Add, subtract, concatenate |
| `=`, `<>`, `<`, `>`, `<=`, `>=` | Comparison |
| `AND` | Bitwise/logical AND |
| `OR`, `XOR` | Bitwise/logical OR, XOR |
| `EQV`, `IMP` | Equivalence, implication |

---

## Special Symbols

| Symbol | Description |
|--------|-------------|
| `:` | Statement separator |
| `;` | Print suppressor |
| `,` | Print tab zone / list separator |
| `#` | File channel prefix |
| `?` | Shorthand for PRINT |
| `'` | Shorthand for REM |
| `&H` | Hex prefix (`&HFF` = 255) |
| `&O` | Octal prefix (`&O77` = 63) |
| `&B` | Binary prefix (`&B1010` = 10) |
| `>` | Redirect to file (overwrite) |
| `>>` | Redirect to file (append) |
| `\|` | Pipe to command |
