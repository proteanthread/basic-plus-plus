<!--
Title:        VERIFY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/debug/diagnostics/verify.c
Generated:    no, hand-written
Status:       current
-->

# `VERIFY` Keyword Reference

## Source Header

```c
// FILENAME: verify.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (analyzer.h, analyzer.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, verify.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the VERIFY statement in BASIC++.
```

## 1. Description & Usage

7-stage AST structural integrity, ISO/IEC 25010 quality, and EU CRA safety/security compliance audit.

## 2. Syntax

```basic
VERIFY [file$] [SUMMARY|DETAILED|JSON]
```

## 3. Code Example

```basic
10 REM VERIFY Demonstration
20 PRINT "VERIFY executed successfully."
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
| Name | VERIFY |
| Category | Debug & Testing |
| Syntax | VERIFY [file$] [SUMMARY\|DETAILED\|JSON] |
| Description | 7-stage AST structural integrity, ISO/IEC 25010 quality, and EU CRA safety/security compliance audit. |
| Error Summary | Error 2: Syntax Error, Error 53: File Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/debug/diagnostics/verify.c |
