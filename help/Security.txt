# Security and Sandboxing in BASIC++

**Version 1.5.0**

BASIC++ includes a security model that restricts what programs can do. This is essential when running untrusted BASIC code.

---

## 1. Security Levels

| Level | Name | Description |
|-------|------|-------------|
| **0** | Unrestricted | Everything is allowed. Full system access. *(default)* |
| **1** | Cautious | Dangerous memory operations blocked. `PEEK`/`POKE` restricted to virtual memory only. `DEF USR` restricted. |
| **2** | Sandboxed | All I/O restricted. `SHELL`, `SHELL$`, `EXEC` blocked. File operations limited to current directory. No `KILL`, `MKDIR`, `RMDIR`, `CHDIR`. |
| **3** | Locked Down | Only pure computation allowed. No file I/O, no shell, no `PEEK`/`POKE`. Only `PRINT` to console. No `ENVIRON$`. Maximum isolation. |

---

## 2. Setting Security Level

```basic
SECURITY LEVEL 0         ' Unrestricted
SECURITY LEVEL 1         ' Cautious
SECURITY LEVEL 2         ' Sandboxed
SECURITY LEVEL 3         ' Locked down

SANDBOX                  ' Shortcut for SECURITY LEVEL 2
```

Security can be set in direct mode or in a program:

```basic
10 SECURITY LEVEL 2
20 ' Rest of program runs sandboxed
```

> **Important:** Security level can only be **increased**, never decreased, within a program. Once you sandbox, you cannot un-sandbox. Only a `NEW` or direct-mode command at level 0 can lower the level.

---

## 3. Function Safety Categories

Every built-in function has a safety classification:

| Category | Description | Blocked At | Examples |
|----------|-------------|------------|----------|
| `FSAFE_PURE` | No side effects. Always allowed. | Never | `ABS`, `LEN`, `LEFT$`, `MID$`, `SIN`, `COS`, `VAL`, `STR$` |
| `FSAFE_STATE` | Reads or modifies interpreter state. | Level 3 | `RND`, `FRE`, `TIMER`, `POS`, `CSRLIN` |
| `FSAFE_IO` | Performs I/O operations. | Level 2+ | `SHELL$`, `ENVIRON$`, `OPEN`, `CLOSE` |
| `FSAFE_UNSAFE` | Direct system access. | Level 1+ | `PEEK`, `POKE`, `INP`, `OUT`, `DEF USR` |

---

## 4. What's Blocked at Each Level

| Feature | L0 | L1 | L2 | L3 |
|---------|:--:|:--:|:--:|:--:|
| `PRINT` (console) | ✅ | ✅ | ✅ | ✅ |
| `INPUT` (keyboard) | ✅ | ✅ | ✅ | ✅ |
| Math functions | ✅ | ✅ | ✅ | ✅ |
| String functions | ✅ | ✅ | ✅ | ✅ |
| Arrays / `DIM` | ✅ | ✅ | ✅ | ✅ |
| `FOR`/`NEXT`/`WHILE`/`WEND` | ✅ | ✅ | ✅ | ✅ |
| `GOSUB`/`RETURN` | ✅ | ✅ | ✅ | ✅ |
| `RND` / `RANDOMIZE` | ✅ | ✅ | ✅ | ❌ |
| `TIMER` / `DATE$` / `TIME$` | ✅ | ✅ | ✅ | ❌ |
| `PEEK` / `POKE` | ✅ | ❌ | ❌ | ❌ |
| `INP` / `OUT` | ✅ | ❌ | ❌ | ❌ |
| `DEF USR` / `USR` | ✅ | ❌ | ❌ | ❌ |
| `OPEN` / `CLOSE` | ✅ | ✅ | Limited | ❌ |
| `PRINT #n` / `INPUT #n` | ✅ | ✅ | Limited | ❌ |
| `KILL` / `NAME` | ✅ | ✅ | ❌ | ❌ |
| `MKDIR` / `RMDIR` / `CHDIR` | ✅ | ✅ | ❌ | ❌ |
| `SHELL` / `SHELL$` | ✅ | ✅ | ❌ | ❌ |
| `EXEC` | ✅ | ✅ | ❌ | ❌ |
| `ENVIRON$` | ✅ | ✅ | ❌ | ❌ |
| `SAVE` / `LOAD` / `MERGE` | ✅ | ✅ | Limited | ❌ |
| `CHAIN` | ✅ | ✅ | ❌ | ❌ |

---

## 5. Limited File Access (Level 2)

At level 2, file operations are restricted to:
- **Current directory only** (no path traversal)
- Cannot delete files (`KILL` blocked)
- Cannot create directories (`MKDIR` blocked)
- Cannot change directory (`CHDIR` blocked)
- Read and write allowed in current dir
- `SAVE`/`LOAD`/`MERGE` only in current dir

---

## 6. Use Cases

### A. Running student homework

```basic
10 SECURITY LEVEL 2
20 ' Students can compute but can't damage the system
30 CHAIN "student_program.bas"
```

### B. Online BASIC playground

```basic
SECURITY LEVEL 3
' Only pure computation -- safe for web execution
```

### C. Batch processing with limited I/O

```basic
10 SECURITY LEVEL 1
20 ' Can do file I/O but can't POKE arbitrary memory
30 OPEN "data.txt" FOR INPUT AS #1
40 ' ...process data...
50 CLOSE #1
```

### D. Plugin testing

```basic
10 SECURITY LEVEL 2
20 ' Test untrusted module code safely
30 IMPORT untrusted_module
```

---

## 7. Security Errors

When a blocked operation is attempted:

```
"Permission denied in 50"
```

This is error code **70**. The error can be caught with `ON ERROR GOTO`:

```basic
10 ON ERROR GOTO 100
20 SECURITY LEVEL 2
30 SHELL "dir"              ' Blocked!
40 END
100 IF ERR = 70 THEN PRINT "Not allowed!" : RESUME NEXT
```

---

## 8. Implementation Notes

The security system is implemented in `security.c`/`.h`. It checks the current security level before executing any sensitive operation.

For C module authors: Set the appropriate `FSAFE_*` level on your registered functions. The security system will automatically enforce the restrictions.

| Constant | Blocked At |
|----------|------------|
| `FSAFE_PURE` | Never |
| `FSAFE_STATE` | Level 3 |
| `FSAFE_IO` | Level 2+ |
| `FSAFE_UNSAFE` | Level 1+ |

---

## 9. Best Practices

- Set security level **early** in your program
- Use the **lowest level** that meets your needs
- Remember: level can go **up** but not **down**
- Test at each security level to verify behavior
- Use `ON ERROR GOTO` to handle permission errors gracefully
- For maximum safety, combine `SECURITY LEVEL 3` with a restricted dialect (e.g., Tiny BASIC)
