<!--
Title:        DOEVENTS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/doevents.c
Generated:    no, hand-written
Status:       current
-->

# `DOEVENTS` Keyword Reference

## Source Header

```c
// FILENAME: doevents.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h)
// NEEDS: libengine (doevents.h, eval.h, eval.c, lexer.h, lexer.c, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DOEVENTS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Yields execution to the operating system to process events and message loops.

## 2. Syntax

```basic
DOEVENTS
```

## 3. Code Example

```basic
10 REM DOEVENTS Demonstration
20 PRINT "DOEVENTS executed successfully."
```

## 4. Error Conditions

Error 5: Invalid context for DOEVENTS

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DOEVENTS |
| Category | Program Flow |
| Syntax | DOEVENTS |
| Description | Yields execution to the operating system to process events and message loops. |
| Error Summary | Error 5: Invalid context for DOEVENTS |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/doevents.c |
