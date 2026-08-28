# BASIC++ v6.5.2 Debugging and Testing Guide

## 1. INTERACTIVE DEBUGGING

BASIC++ provides a built-in interactive debugger accessible from the REPL. The debugger operates at the BASIC line level and provides inspection of variables, call stacks, and execution state without external tools.

### Trace Mode

TRON enables line-number trace mode. When active, the interpreter prints the line number in brackets before executing each line:

```basic
> TRON
> RUN
[10][20][30][40]
Ok
> TROFF
```

TROFF disables trace mode. The TRACE statement provides finer control: TRACE ON enables tracing, TRACE OFF disables it, and TRACE STEP enables single-step mode where the interpreter pauses after each line and waits for input.

### Breakpoints

BREAK n sets a breakpoint at line n. When the interpreter reaches that line during RUN, it pauses execution and enters the debugger prompt. From the debugger prompt you can:

- PRINT expr — Evaluate and display an expression.
- VARS — Display all variable values.
- CONT — Continue execution until the next breakpoint or end of program.
- BACKTRACE — Display the call stack (GOSUB and SUB/FUNCTION frames).
- DUMP — Display internal VM state including stack depths and error state.

BREAK LIST shows all active breakpoints. BREAK CLEAR n removes a breakpoint. BREAK CLEAR removes all breakpoints. The maximum breakpoint count is 64 on modern builds, 8 on FreeDOS, and 4 on embedded.

### The DEBUG Command

DEBUG ON enables verbose debug output, which includes lexer token trace, parser decisions, and expression evaluation steps. This produces substantial output and is primarily useful for diagnosing interpreter bugs rather than BASIC program bugs. DEBUG OFF disables it.

### The STOP Statement

STOP pauses program execution and enters the debugger at the current line. Variables are preserved and CONT resumes execution. STOP is equivalent to a breakpoint that is embedded in the source code rather than set interactively.

## 2. THE SELFTEST COMMAND

SELFTEST runs the interpreter's built-in validation suite. It exercises the lexer, parser, expression evaluator, control flow structures, string operations, array operations, error handling, and virtual device layer. SELFTEST is the primary verification mechanism for build correctness.

```basic
> SELFTEST
```

Run from the command line:

```bash
baspp -c "SELFTEST"
bpp -c "SELFTEST"
```

Both baspp and bpp must pass SELFTEST with zero failures. A failure indicates a regression in the interpreter and must be investigated before any other work proceeds.

## 3. THE ASSERT STATEMENT

ASSERT tests a condition and reports an error if it evaluates to false:

```basic
10 X = 42
20 ASSERT X = 42
30 ASSERT X > 0
40 ASSERT X < 100
```

When an ASSERT fails, it reports the line number, the condition text, and the actual values of the operands. ASSERT does not stop program execution by default; it increments the fail counter and continues. Use ASSERT in combination with TEST/ENDTEST blocks for structured testing.

## 4. TEST BLOCKS

TEST and ENDTEST bracket a named test block. The interpreter tracks pass, fail, and total assertion counts within each block:

```basic
10 TEST "Arithmetic Operations"
20   ASSERT 2 + 2 = 4
30   ASSERT 10 / 3 > 3.33
40   ASSERT 10 MOD 3 = 1
50   ASSERT INT(3.7) = 3
60   ASSERT FIX(-3.7) = -3
70 ENDTEST
80 TEST "String Operations"
90   ASSERT LEN("HELLO") = 5
100  ASSERT LEFT$("HELLO", 3) = "HEL"
110  ASSERT MID$("HELLO", 2, 3) = "ELL"
120  ASSERT INSTR("HELLO WORLD", "WORLD") = 7
130 ENDTEST
```

After a TEST block completes, the interpreter prints the test name, the pass count, the fail count, and the total. At program end, a summary of all test blocks is displayed.

## 5. THE VERIFY STATEMENT

VERIFY tests a condition similar to ASSERT but unconditionally stops program execution if the condition is false. Use VERIFY for critical invariants that indicate a fundamental problem if violated:

```basic
10 VERIFY A$ <> ""   ' Stop if input is empty
```

## 6. THE CHECK STATEMENT

CHECK performs a type or state check and reports the result without stopping execution:

```basic
10 CHECK X > 0       ' Reports pass/fail to debug output
```

## 7. REGRESSION TEST ORGANIZATION

Regression tests are BASIC++ programs stored under the tests/ directory. They are organized by category:

```text
tests/
  gwbasic/              — GW-BASIC compatibility tests
    test_print.bas      — PRINT statement variants
    test_for.bas        — FOR/NEXT loops
    test_if.bas         — IF/THEN/ELSE branching
    test_while.bas      — WHILE/WEND loops
    test_select.bas     — SELECT CASE
    test_string.bas     — String functions
    test_array.bas      — Array operations
    test_file.bas       — File I/O
    test_error.bas      — Error handling
    test_sub.bas        — SUB/FUNCTION procedures
    ...
  categories.json       — Maps categories to test files
  keywords.json         — Maps keywords to test files
```

Execute regression tests using baspp directly:

```bash
baspp tests/gwbasic/test_print.bas
baspp tests/gwbasic/test_for.bas
```

Never use Python, Bash, or other external script wrappers to run tests. All automation must be written in BASIC++ itself and executed by the interpreter.

## 8. THE DAP DEBUG SERVER

The baspp standard edition includes a Debug Adapter Protocol (DAP) server that allows external IDEs to connect and debug BASIC++ programs with a graphical interface. The DAP server is implemented in engine/src/debug/dap_server.c and is part of the libstandard library.

## 9. STATE SAVE AND RESTORE

STATESAVE writes the complete VM state (program, variables, arrays, call stacks, execution position) to a file. STATELOAD restores a saved state. This allows suspending and resuming long-running programs across sessions.

## 10. DEBUGGING TIPS

1. Use TRON to trace the execution path when a program produces unexpected results.
2. Use STOP at a suspected problem line and PRINT to inspect variables.
3. Use ASSERT liberally within TEST blocks to create reproducible test cases.
4. Use the ERR variable in an ON ERROR GOTO handler to log which errors occur and where.
5. Use FRE(0) to check available string heap space if you suspect a memory issue.
6. Use VARS to dump all active variables when the program state seems wrong.
7. Use BACKTRACE to inspect the call stack when RETURN produces unexpected jumps.
