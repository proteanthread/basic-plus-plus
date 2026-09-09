<!--
Title:        RESUME
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/resume.c
Generated:    no, hand-written
Status:       current
-->

# `RESUME` Keyword Reference

## Source Header

```c
// FILENAME: resume.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (continue.c, retry.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, resume.h, string.c)
// NEEDS: libengine (task.h, task.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the RESUME statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Resumes program execution after an error-handling routine finishes, or resumes a suspended task/event.

## 2. Syntax

```basic
RESUME [0 | NEXT | line_label | TASK id | EVENT name$]
```

## 3. Code Example

```basic
10 REM RESUME Demonstration
20 PRINT "RESUME executed successfully."
```

## 4. Error Conditions

Error 20: RESUME Without Error, Error 2: Syntax Error

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
| Name | RESUME |
| Category | Event Trapping |
| Syntax | RESUME [0 \| NEXT \| line_label \| TASK id \| EVENT name$] |
| Description | Resumes program execution after an error-handling routine finishes, or resumes a suspended task/event. |
| Error Summary | Error 20: RESUME Without Error, Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/resume.c |
