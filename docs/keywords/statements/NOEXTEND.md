<!--
Title:        NOEXTEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/external/extend.c
Generated:    no, hand-written
Status:       current
-->

# `NOEXTEND` Keyword Reference

## Source Header

```c
// FILENAME: extend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (extend.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the EXTEND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Disables extended syntax restrictions (classic Minimal BASIC variable rules).

## 2. Syntax

```basic
NO EXTEND | NOEXTEND
```

## 3. Code Example

```basic
10 REM NOEXTEND Demonstration
20 PRINT "NOEXTEND executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Compiler Directives
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NOEXTEND |
| Category | Compiler Directives |
| Syntax | NO EXTEND \| NOEXTEND |
| Description | Disables extended syntax restrictions (classic Minimal BASIC variable rules). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/external/extend.c |
