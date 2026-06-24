# Self-Programming with BASIC++

**Version 4.1.1**


---

## Table of Contents

- Runtime Line Manipulation
- Data-Driven Programs
- EVAL-Like Patterns
  - Using ON GOTO/GOSUB for dispatch tables
  - Using arrays as lookup tables
  - Using string functions to build computed values
- CHAIN and MERGE
- RENUM and Program Surgery
- Using SHELL$ for Code Generation
- Shebang Scripts
- Program Introspection

---

BASIC++ programs can examine and modify themselves at runtime. This guide covers self-modifying code, meta-programming, and program introspection.

---

## 1. Runtime Line Manipulation

In BASIC++, programs are stored as text lines in memory. The interpreter does not pre-compile — it re-parses each line every time it executes. This means you can:

- Add lines to a running program
- Delete lines from a running program
- Modify lines from a running program

**Example — a program that writes itself:**

```basic
10 REM This program adds lines at runtime
20 A$ = "100 PRINT " + CHR$(34) + "I was created!" + CHR$(34)
30 REM Insert line 100 (as if typed at the keyboard)
40 PRINT "Self-modification is possible in direct mode"
50 PRINT "but requires CHAIN or MERGE for runtime insertion"
```

> **Note:** True self-modification of running code requires careful use of `MERGE` or `CHAIN` to inject new lines.

---

## 2. Data-Driven Programs

Instead of modifying code, let `DATA` statements drive behavior:

```basic
10 REM Data-driven menu system
20 READ N
30 FOR I = 1 TO N
40   READ LABEL$, ACTION
50   PRINT I; ") "; LABEL$
60 NEXT I
70 INPUT "Choice: "; C
80 RESTORE
90 READ N
100 FOR I = 1 TO C
110   READ LABEL$, ACTION
120 NEXT I
130 ON ACTION GOSUB 1000, 2000, 3000
140 END
500 DATA 3
510 DATA "New Game", 1, "Load Game", 2, "Quit", 3
```

---

## 3. EVAL-Like Patterns

While BASIC++ doesn't have a direct `EVAL` function, you can achieve similar effects by:

### A. Using ON GOTO/GOSUB for dispatch tables

```basic
10 INPUT "Function (1-3): "; F
20 ON F GOSUB 100, 200, 300
```

### B. Using arrays as lookup tables

```basic
10 DIM TABLE(100)
20 FOR I = 0 TO 99
30   TABLE(I) = I * I
40 NEXT I
50 INPUT "Square root of: "; N
60 PRINT TABLE(N)
```

### C. Using string functions to build computed values

```basic
10 INPUT "Expression: "; E$
20 V = VAL(E$)
30 PRINT "Value: "; V
```

---

## 4. CHAIN and MERGE

**CHAIN** loads a new program and starts executing it:

```basic
10 PRINT "Program A"
20 CHAIN "program_b.bas"
```

Variables are preserved across `CHAIN` by default.

**MERGE** loads lines into the current program without clearing:

```basic
10 REM Main program
20 MERGE "subroutines.bas"
30 REM Now subroutine lines are available
40 GOSUB 5000
```

This enables modular programming — keep subroutine libraries in separate files and `MERGE` them as needed.

---

## 5. RENUM and Program Surgery

`RENUM` renumbers all lines, which is useful after inserting many lines into gaps:

```
RENUM              ' Renumber 10, 20, 30, ...
RENUM 1000         ' Start at 1000
RENUM 100, 5       ' Start at 100, step 5
```

`DELETE` removes ranges:

```
DELETE 100-500     ' Remove lines 100 through 500
```

---

## 6. Using SHELL$ for Code Generation

Generate BASIC code from external tools:

```basic
10 A$ = SHELL$("python generate_data.py")
20 REM A$ now contains generated DATA statements
```

Or pipe generated code back into the interpreter:

```basic
10 SHELL "python gen.py > newcode.bas"
20 MERGE "newcode.bas"
30 KILL "newcode.bas"
```

---

## 7. Shebang Scripts

Make BASIC++ programs directly executable on Unix:

```basic
#!/usr/bin/env basicpp
10 PRINT "I'm a script!"
20 PRINT "Args: "; COMMAND$
```

The `#!` line is silently skipped by the loader.

---

## 8. Program Introspection

Examine program state at runtime:

| Command/Function | Description |
|-----------------|-------------|
| `LIST` | Display the program (in direct mode) |
| `FRE(0)` | Free string memory |
| `FRE(-1)` | Free stack space |
| `FRE(-2)` | Free array space |
| `ERR` | Last error number |
| `ERL` | Line where error occurred |
| `POS(0)` | Cursor column |
| `CSRLIN` | Cursor row |
| `TIMER` | Seconds since midnight |
