# `SCOPE` Lexical Scoping & Execution Hook Statement

## 1. BASIC Usage and Keyword Definition

The `SCOPE` statement creates isolated lexical variable environments, manages execution hooks around language statements, or controls modular namespaces in BASIC++.

### Syntax Signatures:
```basic
SCOPE BEGIN [scope_name$]
  ...
SCOPE END
SCOPE PUSH | SCOPE POP
SCOPE HOOK statement_name BEFORE | AFTER GOSUB line_number
```

### Operational Rules:
- **`SCOPE BEGIN / END`**: Localizes variable definitions created within the block; exiting the block automatically deallocates local variables and releases string handles.
- **`SCOPE HOOK`**: Installs pre-execution or post-execution callback hooks around designated statements.
- **Namespace Isolation**: Prevents accidental global variable collisions across large multi-module codebases.

---

## 2. Code Examples

```basic
10 GLOBAL_X = 100
20 SCOPE BEGIN
30   LOCAL_X = 50 : REM Exists only within this scope block
40   PRINT "Inside scope: LOCAL_X = "; LOCAL_X
50 SCOPE END
60 PRINT "Outside scope: GLOBAL_X = "; GLOBAL_X
```
