# `EXEC` Process Execution Statement

## 1. BASIC Usage and Keyword Definition

The `EXEC` statement spawns an external process or executable file, waits for completion, and returns the exit status code.

### Syntax Signatures:
```basic
EXEC command_line$
exit_code% = EXEC(command_line$)
```

### Operational Rules:
- **Sandbox Gate**: Protected by `CAP_SYS` / `SECOP_EXEC`. In strict sandbox modes, execution is prohibited (Error 70: `ERR_PERMISSION_DENIED`).
- **Platform Abstraction**: Invokes `platform_execute_command()` without raw OS system dependencies.

---

## 2. Code Examples

```basic
10 STATUS% = EXEC("git --version")
20 PRINT "Process exited with status code: "; STATUS%
```
