<!--
Title:        VERSION
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/version.c
Generated:    no, hand-written
Status:       current
-->

# `VERSION` Keyword Reference

## Source Header

```c
// FILENAME: version.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, libcore, libengine, libkernel, libstandard
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c, version.h)
// Provides runtime implementation for the VERSION statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Tags or queries active program version metadata and host engine version.

## 2. Syntax

```basic
VERSION "x.y.z" or VERSION$ / VER$
```

## 3. Code Example

```basic
10 REM VERSION Demonstration
20 PRINT "VERSION executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Environ
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VERSION |
| Category | System & Environ |
| Syntax | VERSION "x.y.z" or VERSION$ / VER$ |
| Description | Tags or queries active program version metadata and host engine version. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/version.c |
