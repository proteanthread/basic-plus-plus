# `SHELL` Command Line Execution Statement

## 1. BASIC Usage and Keyword Definition

The `SHELL` statement spawns the host operating system's command interpreter (e.g. `cmd.exe`/PowerShell on Windows, `/bin/sh` or `/bin/bash` on Linux/macOS) or runs a specified shell command line.

### Syntax Signatures:
```basic
SHELL
SHELL command_string$
```

### Operational Rules:
- **Interactive Shell**: Executing `SHELL` without arguments opens an interactive sub-shell; typing `exit` returns control back to the BASIC++ program.
- **Command Execution**: Executing `SHELL "cmd"` executes the command string, displays output to the console, and resumes the BASIC++ program.
- **Security Check**: Enforced via `CAP_SYS` / `SECOP_EXEC`.

---

## 2. Code Examples

```basic
10 PRINT "Listing current directory via host shell:"
20 SHELL "dir"
30 PRINT "Returned to BASIC++ program execution."
```
