<!--
Title:        GOTO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/goto.c
Generated:    no, hand-written
Status:       current
-->

# `GOTO` Keyword Reference

## Source Header

```c
// FILENAME: goto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, metadata.h, metadata.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, goto.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GOTO statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Unconditionally transfers execution to the specified program line number, expression, or label.

## 2. Syntax

```basic
GOTO line_num | expr | label
```

## 3. Code Example

```basic
10 REM GOTO Demonstration
20 PRINT "GOTO executed successfully."
```

## 4. Error Conditions

Error 8: Undefined line number (target line does not exist), Error 2: Syntax error (missing line number)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GOTO |
| Category | Control Flow |
| Syntax | GOTO line_num \| expr \| label |
| Description | Unconditionally transfers execution to the specified program line number, expression, or label. |
| Error Summary | Error 8: Undefined line number (target line does not exist), Error 2: Syntax error (missing line number) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/goto.c |
