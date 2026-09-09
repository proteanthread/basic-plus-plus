<!--
Title:        SECURITY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/security.c
Generated:    no, hand-written
Status:       current
-->

# `SECURITY` Keyword Reference

## Source Header

```c
// FILENAME: security.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (lexer.h, lexer.c, stmt.h, string.c)
// NEEDS: libkernel (security.h, vdev.h, vdev.c)
// Provides runtime implementation for the SECURITY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Queries or elevates the active engine security sandbox level.

## 2. Syntax

```basic
SECURITY [level_number | LEVEL level_number]
```

## 3. Code Example

```basic
10 REM SECURITY Demonstration
20 PRINT "SECURITY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 70: Permission Denied

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
| Name | SECURITY |
| Category | System & Environ |
| Syntax | SECURITY [level_number \| LEVEL level_number] |
| Description | Queries or elevates the active engine security sandbox level. |
| Error Summary | Error 2: Syntax Error, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/security.c |
