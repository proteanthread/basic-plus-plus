<!--
Title:        GOSUB
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/gosub.c
Generated:    no, hand-written
Status:       current
-->

# `GOSUB` Keyword Reference

## Source Header

```c
// FILENAME: gosub.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c, metadata.h, metadata.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, gosub.h, lexer.h, lexer.c, string.c, vm.h
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GOSUB statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Pushes current line onto stack and branches execution to specified subroutine line number, expression, or label.

## 2. Syntax

```basic
GOSUB line_num | expr | label
```

## 3. Code Example

```basic
10 REM GOSUB Demonstration
20 PRINT "GOSUB executed successfully."
```

## 4. Error Conditions

Error 8: Undefined line number (subroutine target missing), Error 2: Syntax error (missing line number)

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
| Name | GOSUB |
| Category | Control Flow |
| Syntax | GOSUB line_num \| expr \| label |
| Description | Pushes current line onto stack and branches execution to specified subroutine line number, expression, or label. |
| Error Summary | Error 8: Undefined line number (subroutine target missing), Error 2: Syntax error (missing line number) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/gosub.c |
