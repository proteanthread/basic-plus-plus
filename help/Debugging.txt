# Debugging and Tracing in BASIC++

**Version 4.0.1**


---

## Table of Contents

- TRON / TROFF (Trace Mode)
- STOP and CONT
- BREAK (Ctrl+C)
- Variable Inspection
- Single-Stepping Pattern
- Error Debugging
- LIST for Inspection
- Debugging with PRINT
- Debugging File I/O
- Common Errors and Solutions
- Tips
- CHECK and VERIFY (Static Analysis)
- BACKTRACE
- ERROR$ (Error Messages)

---

BASIC++ provides several tools for debugging programs: trace mode, breakpoints, single-stepping, and variable inspection.

---

## 1. TRON / TROFF (Trace Mode)

`TRON` enables trace mode. Every line number is printed in square brackets as it executes:

```basic
TRON                    ' Enable tracing
RUN
```

Output:
```
[10][20][30][40][50]...
```

Each `[nn]` shows the line being executed, letting you follow the program's flow of control.

`TROFF` disables trace mode:

```basic
TROFF                   ' Disable tracing
```

You can toggle tracing from within a program:

```basic
10 TRON
20 PRINT "Traced"
30 TROFF
40 PRINT "Not traced"
```

Output:
```
[10][20]Traced
[30]Not traced
```

---

## 2. STOP and CONT

`STOP` halts execution and drops to direct mode:

```basic
10 PRINT "Before"
20 STOP
30 PRINT "After"
```

```
> RUN
Before
Break in 20
>
```

You're now in direct mode. You can inspect and change variables:

```
> PRINT A
     42
> A = 100
> CONT
```

`CONT` resumes execution from where `STOP` paused.

---

## 3. BREAK (Ctrl+C)

Press **Ctrl+C** during execution to break into direct mode:

```
> RUN
(program is running...)
(press Ctrl+C)
Break in 150
>
```

Then use `CONT` to resume or make changes.

---

## 4. Variable Inspection

In direct mode, you can print any variable:

```
> PRINT A
     42
> PRINT A$
Hello World
> PRINT A(5)
     99
```

You can also change variables:

```
> A = 0
> A$ = "New value"
> CONT
```

---

## 5. Single-Stepping Pattern

BASIC++ doesn't have a built-in single-step command, but you can simulate it with `TRON` and strategic `STOP`s:

```basic
10 TRON
20 A = 1
30 STOP
40 A = A + 1
50 STOP
60 PRINT A
70 TROFF
```

Or add a debug subroutine:

```basic
9000 REM Debug: press Enter to continue
9010 TRON
9020 INPUT "Step>"; D$
9030 RETURN
```

Then in your code:

```basic
100 GOSUB 9000
110 A = A + 1
120 GOSUB 9000
130 B = B + 1
```

---

## 6. Error Debugging

When an error occurs without `ON ERROR GOTO`:

```
HOW? in 150
```

This tells you:
- The error type (`HOW?` = logic error, `WHAT?` = syntax)
- The line number (150)

For more detail:

```
> PRINT ERR
     11
> PRINT ERL
     150
```

`ERR 11` = Division by zero. See [Error_Handling](Error_Handling.md) for the complete error code table.

---

## 7. LIST for Inspection

After a `STOP` or break, use `LIST` to see the code around the current position:

```
> LIST 145-155
145 A = X / Y
150 PRINT A * B
155 GOTO 200
```

---

## 8. Debugging with PRINT

The classic debugging technique: add `PRINT` statements.

```basic
100 PRINT "DEBUG: A="; A; " B="; B
110 C = A / B
120 PRINT "DEBUG: C="; C
```

For conditional debugging, use a flag:

```basic
5 DEBUG = 1    ' Set to 0 to disable
...
100 IF DEBUG THEN PRINT "A="; A; " B="; B
110 C = A / B
120 IF DEBUG THEN PRINT "C="; C
```

---

## 9. Debugging File I/O

When debugging file operations:

```basic
PRINT IOCTL$(#1)       ' Channel mode (I/O/A/R/B)
PRINT LOC(1)           ' Current position
PRINT LOF(1)           ' File size
PRINT EOF(1)           ' At end? (-1 or 0)
```

---

## 10. Common Errors and Solutions

| Error | Meaning | Check |
|-------|---------|-------|
| `WHAT? in 50` | Syntax error at line 50 | Spelling, missing quotes, unmatched parentheses |
| `HOW? in 100` | Logic error at line 100 | Division by zero, bad channel number, file not found |
| `SORRY in 200` | Resource error at line 200 | Out of memory, too many open files, stack overflow |
| Program hangs | Infinite loop | Press Ctrl+C, then `TRON` + `CONT` to find the loop |

---

## 11. Tips

- Use `TRON` sparingly — it produces a LOT of output
- `STOP` before a suspected problem area
- `CONT` to resume; no need to re-`RUN`
- Variables persist between `STOP` and `CONT`
- Ctrl+C works even in tight loops
- `PRINT ERR; ERL` after an error for diagnosis
- Add `REM` comments to document expected values:

```basic
100 REM expect: A=5, B>0
110 C = A / B
```

---

## 12. CHECK and VERIFY (Static Analysis)

Use `CHECK` and `VERIFY` to find problems **before** running your program:

```
> CHECK                    ' Analyze current program
> CHECK "mygame.bas"       ' Analyze a file
> VERIFY "mygame.bas"      ' Deep analysis (CHECK + more)
```

They report:
- Missing `NEXT` for `FOR` loops
- `GOTO`/`GOSUB` targets that don't exist
- Variables used but never assigned
- Memory usage and complexity score

This is faster than adding `PRINT` statements — run `CHECK` first to catch obvious issues.

---

## 13. BACKTRACE

After a `STOP` or Ctrl+C, `BACKTRACE` shows how you got here:

```
> BACKTRACE
[3] line 500 (GOSUB from 200)
[2] line 300 (GOSUB from 100)
[1] line 100 (main)
```

Essential for debugging deeply nested `GOSUB` chains.

---

## 14. ERROR$ (Error Messages)

`ERROR$(n)` translates error codes to readable messages:

```basic
100 ON ERROR GOTO 900
110 X = 1 / 0
900 PRINT "Error "; ERR; ": "; ERROR$(ERR)
910 RESUME NEXT
```

Output: `Error 11: Division by zero`
