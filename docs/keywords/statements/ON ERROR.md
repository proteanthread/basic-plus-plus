<!--
Title:        ON ERROR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/on_error.c
Generated:    no, hand-written
Status:       current
-->

# `ON ERROR` Keyword Reference

## Source Header

```c
// FILENAME: on_error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_error.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the ON_ERROR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables error-trapping routine and specifies the first statement of the handler.

## 2. Syntax

```basic
ON ERROR GOTO {line_label | 0}
```

## 3. Code Example

```basic
10 REM ON ERROR Demonstration
20 PRINT "ON ERROR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON ERROR |
| Category | Event Trapping |
| Syntax | ON ERROR GOTO {line_label \| 0} |
| Description | Enables error-trapping routine and specifies the first statement of the handler. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/on_error.c |
