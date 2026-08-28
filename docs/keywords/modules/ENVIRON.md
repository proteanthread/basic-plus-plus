# `ENVIRON` Environment Variable Setter Statement

## 1. BASIC Usage and Keyword Definition

The `ENVIRON` statement sets, modifies, or deletes an environment variable in the process environment table.

### Syntax Signatures:
```basic
ENVIRON "name=value"
ENVIRON "name="      : REM Deletes environment variable
```

### Operational Rules:
- **Assignment Format**: Parameter string must contain an equals sign (`=`).
- **Deletion**: Specifying an empty value after the equals sign deletes the environment variable.
- **Process Scope**: Changes affect the current BASIC++ process and child processes spawned via `SHELL` or `EXEC`.

---

## 2. Code Examples

```basic
10 ENVIRON "BASICPP_MODE=PRODUCTION"
20 PRINT "Environment variable set: "; ENVIRON$("BASICPP_MODE")
30 ENVIRON "BASICPP_MODE=" : REM Clear variable
```
