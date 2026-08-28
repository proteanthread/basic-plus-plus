# BASIC++ v6.5.2 User's Guide

## 1. STARTING BASIC++

BASIC++ ships as three separate executables. The standard desktop edition is `baspp` on Linux and `baspp.exe` on Windows. It provides the full language with graphics, sound, the TUI editor multiplexer, and all optional subsystems. The lite edition is `bpp` on Linux and `bpp.exe` on Windows. It provides a headless REPL suitable for terminal environments, IoT devices, and server-side scripting without graphics, sound, the TUI editor, or segmented virtual memory. The batch script runner is `bs` on Linux and `bs.exe` on Windows. It executes a BASIC++ source file non-interactively and exits, producing no banner, no prompt, and no REPL.

To start the standard edition, run `baspp` from a terminal or command prompt. The interpreter prints a startup banner identifying the edition, version, and available memory, followed by a ready indicator and the command prompt:

```text
BASIC++ Standard Edition v6.5.2
640 MB RAM Available.

Ok
>
```

The lite edition displays a different banner and prompt style modeled after the Apple II and Commodore conventions:

```text
BASIC++ Lite Edition v6.5.2
384 MB RAM Available.

Ready.
]
```

The batch runner produces no output of its own. Invoke it with a filename argument: `bs myscript.bas`. The exit code is zero on success and non-zero on error. Diagnostic messages from unhandled errors are sent to stderr.

## 2. ENTERING AND RUNNING PROGRAMS

BASIC++ programs consist of numbered lines. To enter a program, type a line number followed by a statement at the prompt:

```basic
> 10 PRINT "HELLO, WORLD!"
> 20 END
```

Each line is stored in program memory when you press Enter. The interpreter responds silently with just the prompt character, suppressing the Ok or Ready indicator for stored lines. To execute the program, type RUN:

```basic
> RUN
HELLO, WORLD!
Ok
>
```

After RUN completes, the interpreter prints Ok (standard edition) or Ready. (lite edition) and returns to the prompt. Immediate statements — those entered without a line number — execute instantly. Typing `PRINT 42` at the prompt prints 42 and then prints the ready indicator.

To view the stored program, type LIST. To clear the program from memory, type NEW. To remove a single line, type its line number alone with no statement. To delete a range of lines, use DELETE followed by the range: `DELETE 100-200`.

Line numbers range from 1 to 65529. The maximum line length is 255 characters. The AUTO command generates line numbers automatically: `AUTO 100,10` starts numbering at 100 with a step of 10. Press Ctrl+C or type a period to exit AUTO mode.

## 3. SAVING AND LOADING PROGRAMS

SAVE writes the current program to a file. LOAD reads a program file into memory, replacing any existing program. Both commands accept a quoted filename:

```basic
> SAVE "MYPROGRAM.BAS"
> LOAD "MYPROGRAM.BAS"
```

MERGE reads a program file and overlays its lines onto the existing program without clearing variables or the execution stack. Lines in the merged file replace existing lines with the same number; lines with new numbers are inserted at the correct position.

CHAIN loads and immediately runs a program file, optionally preserving variables marked with the COMMON statement. CHAIN with the ALL option preserves all active variables. RUN with a filename argument loads and runs the file, clearing all variables first unless the R option is specified: `RUN "MYPROGRAM.BAS", R` preserves COMMON variables and open file descriptors.

BSAVE writes the program in a compact bytecode format. BLOAD reads a bytecode file back into memory. BRUN compiles the current program to bytecode and executes it directly.

## 4. IMMEDIATE MODE

Statements entered without a line number execute immediately and are not stored in the program. Immediate mode is used for testing expressions, inspecting variables, and issuing system commands. Typing `PRINT A` displays the current value of variable A. Typing `A = 42` assigns the value immediately.

Some statements are restricted from immediate mode. DATA and DEF FN must appear in numbered program lines. The interpreter reports Error 12 (Illegal direct statement) if you attempt to use them at the prompt.

After a program stops — whether from an END statement, a STOP statement, an error, or Ctrl+C — the variables remain intact. You can inspect them with PRINT or with the VARS command, which dumps all active variable names and values. The CONT command resumes execution from where it stopped, provided you have not modified the program. If a line has been added, changed, or deleted since the stop, CONT reports Error 17 (Cannot continue).

## 5. DATA TYPES

BASIC++ uses double-precision floating-point numbers as its primary numeric type. All numeric variables, expressions, and constants operate in double precision. Integer arithmetic is available through the CINT, FIX, and INT functions, which convert values to integer representation without changing the underlying storage type.

String values are sequences of characters stored in a reference-counted string heap managed by the StringContext subsystem. The maximum length of a single string is 255 characters. String variable names end with a dollar sign: `A$`, `NAME$`, `GREETING$`. String concatenation uses the plus operator: `A$ = "HELLO" + " " + "WORLD"`.

Variable names consist of a letter followed by up to 30 additional alphanumeric characters. The name is case-insensitive: `Score`, `SCORE`, and `score` refer to the same variable. A type suffix character declares the variable type explicitly: `%` for integer, `!` for single precision, `#` for double precision, and `$` for string. Without a suffix, the type is determined by the active DEFINT, DEFSNG, DEFDBL, or DEFSTR declarations, defaulting to double precision if none are set.

## 6. OPERATORS AND EXPRESSIONS

Arithmetic operators are `+` (addition), `-` (subtraction), `*` (multiplication), `/` (division), `\` (integer division), `^` (exponentiation), and `MOD` (modulus). Comparison operators are `=` (equal), `<>` (not equal), `<` (less than), `>` (greater than), `<=` (less than or equal), and `>=` (greater than or equal). Logical and bitwise operators are `AND`, `OR`, `NOT`, `XOR`, `EQV` (equivalence), and `IMP` (implication). Shift operators are `SHL` (shift left) and `SHR` (shift right).

Operator precedence from highest to lowest is: exponentiation, unary minus, multiplication and division, integer division, modulus, addition and subtraction, comparison operators, NOT, AND, OR, XOR, EQV, and IMP. Parentheses override precedence as expected.

String comparison is lexicographic and case-sensitive. Comparing a string to a number produces Error 13 (Type mismatch).

## 7. CONTROL FLOW

BASIC++ provides both line-number-based and structured control flow.

IF...THEN...ELSE evaluates a condition and branches accordingly. The single-line form places the THEN clause and optional ELSE clause on one line: `IF A > 10 THEN PRINT "BIG" ELSE PRINT "SMALL"`. The block form uses ELSEIF and END IF on separate lines:

```basic
10 IF A > 100 THEN
20   PRINT "LARGE"
30 ELSEIF A > 10 THEN
40   PRINT "MEDIUM"
50 ELSE
60   PRINT "SMALL"
70 END IF
```

FOR...NEXT loops iterate a variable through a range: `FOR I = 1 TO 10 STEP 2`. The STEP clause defaults to 1 if omitted. The BY keyword is an alternative spelling of STEP inherited from Tymshare Super BASIC. EXIT FOR leaves the loop early.

WHILE...WEND loops execute while a condition is true. DO...LOOP provides pre-test and post-test variants: `DO WHILE condition`, `DO UNTIL condition`, `LOOP WHILE condition`, and `LOOP UNTIL condition`. EXIT DO leaves a DO loop early.

SELECT CASE evaluates an expression and matches it against CASE clauses:

```basic
10 SELECT CASE X
20   CASE 1
30     PRINT "ONE"
40   CASE 2, 3
50     PRINT "TWO OR THREE"
60   CASE IS > 10
70     PRINT "GREATER THAN TEN"
80   CASE ELSE
90     PRINT "SOMETHING ELSE"
100 END SELECT
```

GOTO transfers execution to a line number. GOSUB transfers execution to a subroutine at a line number and RETURN jumps back to the statement after the GOSUB. ON...GOTO and ON...GOSUB select a target from a list based on a numeric expression: `ON X GOTO 100, 200, 300`.

The postfix IF modifier places a condition after a statement: `PRINT "OK" IF A = 1`. The UNLESS modifier is a negated postfix IF: `PRINT "MISSING" UNLESS A > 0`. Multiple postfix conditions evaluate right to left; any false condition aborts the statement.

## 8. SUBROUTINES AND FUNCTIONS

SUB defines a named subroutine that accepts parameters and has its own local variable scope. CALL invokes a SUB by name:

```basic
100 SUB GreetUser(name$)
110   PRINT "Hello, "; name$
120 END SUB
...
200 CALL GreetUser("Alice")
```

FUNCTION defines a named function that returns a value. The return value is assigned to the function name within the body:

```basic
100 FUNCTION Square(x)
110   Square = x * x
120 END FUNCTION
...
200 PRINT Square(7)
```

DEF FN defines a single-expression user function in the classic BASIC style: `DEF FN AREA(R) = PI * R * R`. The function is invoked as `FN AREA(5)`.

Variables inside SUB and FUNCTION are local by default. The SHARED statement makes a main-program variable accessible inside a procedure. The STATIC statement preserves a local variable's value across calls. The LOCAL statement explicitly declares a variable as local.

## 9. ARRAYS AND DATA

DIM declares an array with specified dimensions: `DIM A(100)` creates a one-dimensional array with elements 0 through 100 (101 elements by default). Up to three dimensions are supported: `DIM GRID(10, 20, 5)`. OPTION BASE 0 and OPTION BASE 1 control whether array indices start at 0 or 1.

REDIM resizes a dynamic array. REDIM PRESERVE resizes without clearing existing elements. ERASE deletes an array and reclaims its memory. LBOUND and UBOUND return the lower and upper bounds of an array dimension.

DATA, READ, and RESTORE provide inline data storage. DATA statements contain comma-separated values. READ assigns the next DATA value to a variable. RESTORE resets the DATA pointer to the beginning or to a specific line number: `RESTORE 500`.

MAT operations perform matrix arithmetic on two-dimensional arrays. MAT IDN fills a matrix with the identity pattern. MAT ZER fills a matrix with zeros. MAT TRN transposes a matrix. MAT INV computes the inverse. MAT assignments support addition, subtraction, and scalar multiplication.

## 10. FILE OPERATIONS

OPEN establishes a connection between a file and a channel number:

```basic
OPEN "DATA.TXT" FOR INPUT AS #1
OPEN "OUTPUT.TXT" FOR OUTPUT AS #2
OPEN "LOG.TXT" FOR APPEND AS #3
OPEN "RECORDS.DAT" FOR RANDOM AS #4 LEN = 128
```

The four file modes are INPUT (reading), OUTPUT (writing from the beginning), APPEND (writing at the end), and RANDOM (fixed-length record access). Up to 16 files can be open simultaneously.

INPUT #n reads data from a file channel. PRINT #n writes to a file channel. LINE INPUT #n reads an entire line including commas. WRITE #n writes comma-delimited data with strings in quotes.

For random access files, FIELD defines named buffer fields within a record. GET #n reads a record by number. PUT #n writes a record by number. LSET and RSET left-justify and right-justify strings in field buffers. CVI, CVS, CVD convert field strings to numeric values. MKI$, MKS$, MKD$ convert numbers to field strings.

EOF(n) returns true when the end of file n has been reached. LOC(n) returns the current position. LOF(n) returns the file length. SEEK repositions the file pointer.

CLOSE closes one or more channels. RESET closes all open files. LOCK and UNLOCK provide record-level file locking.

## 11. SCREEN AND CONSOLE

CLS clears the screen. LOCATE positions the cursor: `LOCATE row, col`. COLOR sets the foreground and background colors: `COLOR 15, 1` sets white text on blue. WIDTH sets the screen width in columns: `WIDTH 80`.

CSRLIN returns the current cursor row. POS(0) returns the current cursor column. SCREEN selects a graphics mode or text mode. The SCREEN command accepts mode numbers compatible with GW-BASIC and QBASIC screen mode conventions.

BEEP produces an audible tone. SOUND plays a tone at a specified frequency and duration: `SOUND 440, 18.2` plays concert A for one second (18.2 ticks per second). PLAY interprets a music macro language string for melodic sequences.

KEY assigns text strings to function keys for quick entry. KEY ON displays the function key assignments at the bottom of the screen. KEY OFF hides them. KEY LIST shows all assignments.

## 12. GRAPHICS

PSET sets a single pixel: `PSET (x, y), color`. PRESET resets a pixel to the background color. LINE draws a line or filled rectangle: `LINE (x1,y1)-(x2,y2), color, BF`. CIRCLE draws a circle or arc: `CIRCLE (x,y), radius, color`. PAINT flood-fills an area: `PAINT (x,y), fill_color, border_color`.

DRAW interprets a graphics macro language for turtle-style drawing. PALETTE remaps color indices. POINT reads the color of a pixel. VIEW defines a graphics viewport. WINDOW sets the logical coordinate system. PCOPY copies a display page to another page.

The BGI (BASIC++ Graphics Interface) extends classic screen modes with custom resolution initializers. SET SCREEN accepts width, height, bits per pixel, text column count, and text row count. SET GRAPHICS provides a split-screen mode with a text window area.

## 13. ERROR HANDLING

ON ERROR GOTO establishes a global error handler. When a runtime error occurs, execution transfers to the specified line. Inside the handler, ERR returns the error code and ERL returns the line number where the error occurred. ERR$ returns the error message text. RESUME returns from the handler to retry the failing statement. RESUME NEXT skips the failing statement. RESUME 0 retries, and RESUME with a line number jumps to that line.

ON ERROR GOTO 0 disables error trapping and restores default error reporting. The ERROR statement deliberately triggers a runtime error: `ERROR 5` raises Illegal function call.

BASIC++ error codes follow the GW-BASIC/QBASIC numbering. Common error codes include Error 2 (Syntax error), Error 5 (Illegal function call), Error 6 (Overflow), Error 7 (Out of memory), Error 9 (Subscript out of range), Error 11 (Division by zero), Error 13 (Type mismatch), Error 53 (File not found), and Error 70 (Permission denied).

## 14. THE HELP SYSTEM

The HELP command displays interactive documentation for any keyword. Type `HELP PRINT` to see the reference for PRINT. Type `HELP` alone for a topic list. The CATALOG command lists all available keywords, functions, and statements organized by category.

The INFO command displays system information including the version, dialect, memory configuration, and platform details. The VER command prints the version string. VER$ returns it as a string value.

## 15. DIALECTS

BASIC++ supports multiple BASIC dialects. The default dialect is GW-BASIC. Other available dialects include QBASIC, ECMA-116 Full BASIC, Tymshare Super BASIC, Palo Alto Tiny BASIC, ECMA-55 Minimal BASIC, Sinclair BASIC, SuperBASIC (QL), and Apple II Applesoft BASIC.

Switch dialects at runtime with the DIALECT command: `DIALECT "QBAS"`. Set the dialect from the command line with the `-d` flag: `baspp -d QBAS`. Each dialect defines its own keyword set, syntax rules, and numeric behavior. Programs written for one dialect execute correctly when that dialect is active, and the VM executes dialect-agnostic bytecode regardless of which dialect parsed the source.

## 16. COMMAND-LINE OPTIONS

The standard edition accepts the following command-line arguments:

`baspp` starts the interactive REPL.
`baspp filename.bas` loads and enters the REPL with the program in memory.
`baspp -c "statement"` executes a single statement and exits.
`baspp -d CODE` sets the active dialect (e.g., GWBS, QBAS, E116).
`baspp --log` enables file logging. `baspp --log=path` specifies a log file path.
`baspp --debug` enables debug-level logging. `baspp --trace` enables trace-level logging.

Without `--log`, `--debug`, or `--trace`, no log files are generated on disk.

## 17. SECURITY LEVELS

BASIC++ enforces a security model with six levels that control access to system resources. The levels from least restrictive to most restrictive are OPEN (all operations permitted), SAFE (secure but functional), STANDARD (controlled sandbox), EDUCATIONAL (classroom mode), RESTRICTED (very limited), and PARANOID (pure computation only).

The SECURITY command queries and changes the active security level: `SECURITY LEVEL 2` sets STANDARD mode. Operations that violate the active security policy produce Error 70 (Permission denied). The RESTRICT command disables specific operations or keywords independently of the overall security level.

## 18. EXITING

END terminates program execution and returns to the prompt. STOP pauses execution at the current line, allowing inspection of variables and continuation with CONT. SYSTEM or BYE exits the interpreter entirely and returns to the operating system. In the standard edition, typing SYSTEM at the Ok prompt closes the interpreter. In the batch runner, the interpreter exits automatically after the script finishes.
