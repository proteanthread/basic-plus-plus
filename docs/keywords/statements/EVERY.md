<!--
Title:        EVERY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_every.c
Generated:    no, hand-written
Status:       current
-->

# `EVERY` Keyword Reference

## Source Header

```c
// FILENAME: stmt_every.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the EVERY statement for periodic timer event scheduling.
//
// ---- Includes ----
```

## 1. Description & Usage

Schedules a recurring subroutine execution on hardware timer intervals.

## 2. Syntax

```basic
EVERY ms GOSUB line_num
```

## 3. Code Example

```basic
10 REM EVERY Demonstration
20 PRINT "EVERY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Timing & Real-Time
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EVERY |
| Category | Timing & Real-Time |
| Syntax | EVERY ms GOSUB line_num |
| Description | Schedules a recurring subroutine execution on hardware timer intervals. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_every.c |
