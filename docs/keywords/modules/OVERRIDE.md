# `OVERRIDE` Statement Handler Interception Statement

## 1. BASIC Usage and Keyword Definition

The `OVERRIDE` statement allows developers to intercept and replace the built-in C execution handler of any standard BASIC++ statement or command with a custom user-defined BASIC++ subroutine (`GOSUB` target line or `SUB` block).

### Syntax Signatures:
```basic
OVERRIDE keyword WITH GOSUB line_number
OVERRIDE keyword WITH SUB procedure_name
OVERRIDE RESTORE keyword
OVERRIDE LIST
```

### Operational Rules:
- **Statement Interception**: When the overridden keyword is executed, the VM redirects execution to the user subroutine, passing statement tokens/arguments into a local context.
- **RESTORE**: Restores the original C engine execution handler.
- **Protected Core**: Fundamental control flow statements (`OVERRIDE`, `END`, `STOP`, `RUN`) cannot be intercepted.

---

## 2. Code Examples

```basic
10 OVERRIDE CLS WITH GOSUB 1000
20 CLS : REM Executes custom handler at line 1000
30 END

1000 REM Custom CLS handler
1010 PRINT "[Custom Screen Clear Executed]"
1020 RETURN
```
