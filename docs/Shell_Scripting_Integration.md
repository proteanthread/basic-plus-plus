<!--
Title:        Shell_Scripting_Integration
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/bootstrap/bs/bs.c, engine/src/statements/system/stmt_shell.c, engine/src/eval/functions/system/environment/func_exec.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Shell Scripting Integration Architecture

The authoritative specification for headless batch execution, Unix pipelines, operating system command dispatch, and environment integration in BASIC++ v6.5.2.

---

## 1. The Batch Script Runner (`bs` / `bs.exe`)

The `bs` executable is specifically designed for non-interactive shell scripting, automated tasks, and CI/CD pipelines:

- **Zero Banner & Zero Prompt**: Produces no startup banner or interactive prompt, ensuring clean stdout streams.
- **Deterministic Exit Codes**:
  - `0`: Successful execution.
  - `1`: Runtime error.
  - `2`: Script file not found.
  - `3`: Syntax or compilation error.
- **Pipeline Invocation**:
  ```bash
  bs script.bas
  cat data.csv | bs filter.bas > output.csv
  ```

---

## 2. Standard I/O Pipelines

The batch runner attaches directly to standard OS streams:
- `INPUT` and `LINE INPUT` read from standard input (`stdin`).
- `PRINT` and `WRITE` emit data to standard output (`stdout`).
- Diagnostics, assertions, and error messages are directed to standard error (`stderr`).

```basic
10 REM Filter stdin to stdout
20 ON ERROR GOTO 100
30 WHILE 1
40   LINE INPUT LineData$
50   PRINT UCASE$(LineData$)
60 WEND
100 END
```

---

## 3. Operating System Command Execution

BASIC++ provides multiple complementary mechanisms for interacting with the host operating system:

- **`SHELL "command"`**: Executes a command string via the host command interpreter (`cmd.exe` or `/bin/sh`). Execution halts until the command completes.
- **`SHELL$("command")`**: Executes an OS shell command and captures its standard output as a string.
- **`EXEC$(binary$ [, arg1$ [, arg2$...]])`**: Spawns an external binary directly with argument vector passing, bypassing shell interpretation for safety and performance.
- **`ERRORLEVEL`**: Built-in system variable containing the integer exit code of the most recently executed `SHELL` or `EXEC$` process.

---

## 4. Environment Variables and Process Termination

- **`ENVIRON$("VAR")`**: Reads the value of an environment variable.
- **`ENVIRON "VAR=VALUE"`**: Sets or modifies an environment variable in the active process environment.
- **`COMMAND$`**: Returns the raw command-line argument string passed to the script.
- **`SYSTEM [exit_code]`**: Immediately terminates the interpreter process, returning `exit_code` to the parent shell.
