<!--
Title:        CHECK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/debug/diagnostics/check.c
Generated:    no, hand-written
Status:       current
-->

# `CHECK` Keyword Reference

## Source Header

```c
// FILENAME: check.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (analyzer.h, analyzer.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (check.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the CHECK statement in BASIC++.
```

## 1. Description & Usage

Executes 7-stage code health audit, keyword-variable conflict detection, and semantic inference.

## 2. Syntax

```basic
CHECK [file$] [SUMMARY|DETAILED|JSON|CONFLICTS|SMART]
```

## 3. Code Example

```basic
10 REM CHECK Demonstration
20 PRINT "CHECK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 53: File Not Found

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
| Name | CHECK |
| Category | Debug & Testing |
| Syntax | CHECK [file$] [SUMMARY\|DETAILED\|JSON\|CONFLICTS\|SMART] |
| Description | Executes 7-stage code health audit, keyword-variable conflict detection, and semantic inference. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/debug/diagnostics/check.c |
