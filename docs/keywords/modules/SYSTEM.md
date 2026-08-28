# `SYSTEM` Engine Exit and Diagnostics Statement

## 1. BASIC Usage and Keyword Definition

The `SYSTEM` statement terminates the BASIC++ program execution and exits back to the host operating system shell, or when passed a query argument, displays runtime platform and compiler diagnostic information.

### Syntax Signatures:
```basic
SYSTEM
SYSTEM "PLATFORM"
SYSTEM "VERSION"
SYSTEM "MEMORY"
SYSTEM "COMPILER"
SYSTEM "WORDSIZE"
```

### Operational Rules:
- **`SYSTEM` (No Arguments)**: Closes open file channels, releases device allocations, flushes output buffers, and exits the process with return code `0`.
- **`SYSTEM query$`**: Queries runtime metadata without exiting the interpreter.

---

## 2. Code Examples

```basic
10 PRINT "Querying BASIC++ runtime diagnostics:"
20 SYSTEM "VERSION"
30 SYSTEM "MEMORY"
40 SYSTEM "COMPILER"
50 SYSTEM : REM Exit back to host OS
```
