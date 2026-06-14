# BASIC++ Debugging and Testing

**Version 2.0.0**

This guide covers all debugging and testing features built into BASIC++. These tools help you find bugs, verify behavior, and build confidence in your programs.

---

## Trace Mode (TRON / TROFF)

`TRON` enables line-number tracing. As each line executes, its number is printed in square brackets:

```
TRON
RUN
[10][20][30][40][50]
```

`TROFF` disables tracing.

Trace output goes to the console (STDOUT) and is interleaved with normal `PRINT` output. This is useful for finding infinite loops, unexpected `GOTO`/`GOSUB` paths, and confirming execution order.

**Example:**

```basic
10 TRON
20 FOR I = 1 TO 3
30 PRINT I
40 NEXT I
50 TROFF
```

Output:
```
[10][20][30] 1
[40][20][30] 2
[40][20][30] 3
[40][50]
```

---

## Breakpoints (BREAK)

`BREAK` sets a breakpoint on a specific line number. When execution reaches that line, the program pauses and drops to direct mode (like `STOP`).

```
BREAK 100          ' Set breakpoint at line 100
BREAK -100         ' Clear breakpoint at line 100
BREAK              ' List all active breakpoints
```

When a breakpoint fires:

```
Break at line 100
>
```

You can examine variables with `VARS` or `PRINT`, then resume with `CONT`.

Multiple breakpoints are supported (up to **16** simultaneously). Breakpoints survive `RUN` but are cleared by `NEW`.

---

## Stopping and Continuing (STOP / CONT)

`STOP` pauses execution and drops to direct mode:

```basic
100 STOP
```

When `STOP` executes:

```
Break in 100
>
```

In direct mode you can:
- `PRINT` variables to inspect state
- Use `VARS` to see all variables
- Modify variables with `LET`
- Resume execution with `CONT`

`CONT` resumes from where the program stopped. It remembers the exact position (line and statement). If you modify the program (add/delete lines), `CONT` may not work correctly — use `RUN` instead.

---

## Variables Inspection (VARS)

`VARS` displays variables and environment information:

| Command | Description |
|---------|-------------|
| `VARS` | Show all program variables |
| `VARS USER` | Show user-defined variables only |
| `VARS ENV` | Show OS environment variables |
| `VARS SYSTEM` | Show BASIC++ system state |
| `VARS ALL` | Show everything |

**VARS USER** output example:
```
[User Variables]
A         = 42
B$        = "Hello"
X(3)      = 99
I         = 10
```

**VARS SYSTEM** output example:
```
[System Variables]
Dialect   = GWBS
Version   = 2.0.0
Memory    = 524288 bytes
Free      = 511420 bytes
```

---

## Assertions (ASSERT)

`ASSERT` tests a condition and reports pass or fail:

```basic
ASSERT 2 + 3 = 5         ' passes
ASSERT A > 0             ' passes if A is positive
ASSERT LEN("HELLO") = 5  ' passes
```

When an assertion **passes**, nothing is printed (silent).
When an assertion **fails**:

```
ASSERT FAILED in line 100: 2 + 3 = 6
```

`ASSERT` is useful for validating function output, checking boundary conditions, and regression testing. In a `TEST` block, `ASSERT` results contribute to the block's pass/fail count.

---

## Test Blocks (TEST / ENDTEST)

`TEST` and `ENDTEST` create named test blocks that collect multiple assertions and report results:

```basic
TEST "Arithmetic"
ASSERT 2 + 3 = 5
ASSERT 10 - 7 = 3
ASSERT 4 * 5 = 20
ENDTEST
```

Output:
```
TEST "Arithmetic": 3 passed, 0 failed
```

If any assertion fails:
```
TEST "Arithmetic": 2 passed, 1 failed ***FAIL***
```

You can have multiple `TEST` blocks in a single program:

```basic
10 TEST "Math"
20 ASSERT ABS(-5) = 5
30 ASSERT INT(3.7) = 3
40 ENDTEST
50 TEST "Strings"
60 ASSERT LEN("ABC") = 3
70 ASSERT LEFT$("HELLO", 3) = "HEL"
80 ENDTEST
```

---

## Built-in Self-Test (SELFTEST)

`SELFTEST` runs the interpreter's internal validation suite. This tests the core subsystems:

```
> SELFTEST
=== BASIC++ SELF-TEST ===
Lexer............. PASS
Value system...... PASS
String pool....... PASS
Function registry. PASS
Memory pool....... PASS
=== 5 passed, 0 failed ===
```

`SELFTEST` is a direct-mode command. It does not affect the current program or variables. Use it after building from source to verify the interpreter is working correctly.

---

## Error Line and Code (ERL / ERR)

After an error occurs (and is caught by `ON ERROR GOTO`):

| Variable | Description |
|----------|-------------|
| `ERL` | Line number where the error occurred |
| `ERR` | Numeric error code |

**Example:**

```basic
10 ON ERROR GOTO 100
20 X = 1 / 0
30 END
100 PRINT "Error"; ERR; "at line"; ERL
110 RESUME NEXT
```

Output:
```
Error 11 at line 20
```

---

## Debugging Workflow

A typical debugging session:

1. `RUN` your program
2. If it hangs, press **Ctrl+C** to break
3. Use `TRON` to enable tracing, then `RUN` again
4. Once you find the problem area, set breakpoints:
   ```
   BREAK 200
   BREAK 350
   ```
5. `RUN` again. When a breakpoint fires, inspect:
   ```
   PRINT X, Y, Z
   VARS USER
   ```
6. `CONT` to resume, or fix the code and `RUN` again
7. Write `ASSERT`s to prevent regressions:
   ```basic
   ASSERT X > 0
   ASSERT LEN(A$) <= 255
   ```
8. Wrap tests in `TEST` blocks for organized reporting

**Tips:**
- Use `BREAK` for targeted debugging
- Use `TRON` for understanding flow
- Use `ASSERT` at the start of subroutines to validate input assumptions
- Use `TEST` blocks to create repeatable test suites
- `SELFTEST` after any source-level changes to the interpreter itself
