<!--
Title:        RELEASE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/release.c
Generated:    no, hand-written
Status:       current
-->

# `RELEASE` Keyword Reference

## Source Header

```c
// FILENAME: release.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, release.h, vm.h)
// Provides runtime implementation for the RELEASE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Releases record locks across all channels, a specific channel, or an individual record.

## 2. Syntax

```basic
RELEASE | RELEASE [#]ch | RELEASE [#]ch, id | RELEASE[ch, id]
```

## 3. Code Example

```basic
10 REM RELEASE Demonstration
20 PRINT "RELEASE executed successfully."
```

## 4. Error Conditions

Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Pick & Business BASIC
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RELEASE |
| Category | Pick & Business BASIC |
| Syntax | RELEASE \| RELEASE [#]ch \| RELEASE [#]ch, id \| RELEASE[ch, id] |
| Description | Releases record locks across all channels, a specific channel, or an individual record. |
| Error Summary | Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/release.c |
