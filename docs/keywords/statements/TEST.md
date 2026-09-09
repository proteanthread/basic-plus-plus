<!--
Title:        TEST
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/debug/diagnostics/test.c
Generated:    no, hand-written
Status:       current
-->

# `TEST` Keyword Reference

## Source Header

```c
// FILENAME: test.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, test.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the TEST statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Begins or ends a declarative unit test suite block and outputs assertion metrics.

## 2. Syntax

```basic
TEST "suite_name" [SUBCASE "case_name"]
```

## 3. Code Example

```basic
10 REM TEST Demonstration
20 PRINT "TEST executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Debug & Testing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TEST |
| Category | Debug & Testing |
| Syntax | TEST "suite_name" [SUBCASE "case_name"] |
| Description | Begins or ends a declarative unit test suite block and outputs assertion metrics. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/debug/diagnostics/test.c |
