<!--
Title:        WRITEU
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/writeu.c
Generated:    no, hand-written
Status:       current
-->

# `WRITEU` Keyword Reference

## Source Header

```c
// FILENAME: writeu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, vm.h, writeu.h)
// Provides runtime implementation for the WRITEU statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Writes a record to a channel or ISAM store while retaining the exclusive update lock.

## 2. Syntax

```basic
WRITEU [#]ch, id, data$ | WRITEU[ch, id] data$ | WRITEU data$ ON [#]ch, id
```

## 3. Code Example

```basic
10 REM WRITEU Demonstration
20 PRINT "WRITEU executed successfully."
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
| Name | WRITEU |
| Category | Pick & Business BASIC |
| Syntax | WRITEU [#]ch, id, data$ \| WRITEU[ch, id] data$ \| WRITEU data$ ON [#]ch, id |
| Description | Writes a record to a channel or ISAM store while retaining the exclusive update lock. |
| Error Summary | Error 52: Bad File Number, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/writeu.c |
