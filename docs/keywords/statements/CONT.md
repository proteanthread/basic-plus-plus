<!--
Title:        CONT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/cont.c
Generated:    no, hand-written
Status:       current
-->

# `CONT` Keyword Reference

## Source Header

```c
// FILENAME: cont.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (cont.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CONT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Resumes program execution after a break or STOP statement.

## 2. Syntax

```basic
CONT
```

## 3. Code Example

```basic
10 REM CONT Demonstration
20 PRINT "CONT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 17: Can't Continue

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CONT |
| Category | Program Mgmt & Editing |
| Syntax | CONT |
| Description | Resumes program execution after a break or STOP statement. |
| Error Summary | Error 2: Syntax Error, Error 17: Can't Continue |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/cont.c |
