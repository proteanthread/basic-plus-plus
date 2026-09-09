<!--
Title:        READU
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/readu.c
Generated:    no, hand-written
Status:       current
-->

# `READU` Keyword Reference

## Source Header

```c
// FILENAME: readu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, readu.h, vm.h)
// Provides runtime implementation for the READU statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads a record from a channel or ISAM store and sets an exclusive update lock.

## 2. Syntax

```basic
READU [#]ch, id, var$ | READU[ch, id] var$ | READU var$ FROM [#]ch, id
```

## 3. Code Example

```basic
10 REM READU Demonstration
20 PRINT "READU executed successfully."
```

## 4. Error Conditions

Error 52: Bad File Number, Error 5: Illegal Function Call

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
| Name | READU |
| Category | Pick & Business BASIC |
| Syntax | READU [#]ch, id, var$ \| READU[ch, id] var$ \| READU var$ FROM [#]ch, id |
| Description | Reads a record from a channel or ISAM store and sets an exclusive update lock. |
| Error Summary | Error 52: Bad File Number, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/readu.c |
