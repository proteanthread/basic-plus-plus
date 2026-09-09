<!--
Title:        VARS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/variable/var.c
Generated:    no, hand-written
Status:       current
-->

# `VARS` Keyword Reference

## Source Header

```c
// FILENAME: var.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for VARS.
```

## 1. Description & Usage

Lists all currently allocated variables, their scopes, data types, and values to the console.

## 2. Syntax

```basic
VARS [prefix$]
```

## 3. Code Example

```basic
10 REM VARS Demonstration
20 PRINT "VARS executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Management & Introspection
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VARS |
| Category | Program Management & Introspection |
| Syntax | VARS [prefix$] |
| Description | Lists all currently allocated variables, their scopes, data types, and values to the console. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/variable/var.c |
