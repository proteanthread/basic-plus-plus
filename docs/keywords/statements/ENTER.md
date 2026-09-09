<!--
Title:        ENTER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/enter.c
Generated:    no, hand-written
Status:       current
-->

# `ENTER` Keyword Reference

## Source Header

```c
// FILENAME: enter.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (enter.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ENTER statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs timed input from terminal or channel with status return (HP 2000 TSB).

## 2. Syntax

```basic
ENTER [#channel,] timeout_sec, status_var, string_var
```

## 3. Code Example

```basic
10 REM ENTER Demonstration
20 PRINT "ENTER executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Input / Output
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ENTER |
| Category | Input / Output |
| Syntax | ENTER [#channel,] timeout_sec, status_var, string_var |
| Description | Performs timed input from terminal or channel with status return (HP 2000 TSB). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/enter.c |
