<!--
Title:        COLLATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/extended/collate.c
Generated:    no, hand-written
Status:       current
-->

# `COLLATE` Keyword Reference

## Source Header

```c
// FILENAME: collate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the COLLATE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets the character collation sequence for string comparisons (ANSI Full BASIC X3.113-1987 Section 9 / ECMA-116).

## 2. Syntax

```basic
COLLATE STANDARD | COLLATE NATIVE | COLLATE table$
```

## 3. Code Example

```basic
10 REM COLLATE Demonstration
20 PRINT "COLLATE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 5: Illegal function call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Strings & Formatting
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | COLLATE |
| Category | Strings & Formatting |
| Syntax | COLLATE STANDARD \| COLLATE NATIVE \| COLLATE table$ |
| Description | Sets the character collation sequence for string comparisons (ANSI Full BASIC X3.113-1987 Section 9 / ECMA-116). |
| Error Summary | Error 2: Syntax error, Error 5: Illegal function call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/extended/collate.c |
