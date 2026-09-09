<!--
Title:        EXTERN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/external/stmt_extern.c
Generated:    no, hand-written
Status:       current
-->

# `EXTERN` Keyword Reference

## Source Header

```c
// FILENAME: stmt_extern.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libengine (eval.h, stmt.h), libplat
// Provides runtime implementation for the dynamic FFI EXTERN statement in BAS
//
// ---- Includes ----
```

## 1. Description & Usage

Dynamically binds an external dynamic shared library (DLL/.so) C symbol for direct calling in BASIC++.

## 2. Syntax

```basic
EXTERN {SUB | FUNCTION} name [ALIAS "aliasname"] [(params)]
```

## 3. Code Example

```basic
10 Val = EXTERN {SUB | FUNCTION} name [ALIAS "aliasname"] [(params)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EXTERN |
| Category | Control Flow |
| Syntax | EXTERN {SUB \| FUNCTION} name [ALIAS "aliasname"] [(params)] |
| Description | Dynamically binds an external dynamic shared library (DLL/.so) C symbol for direct calling in BASIC++. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/external/stmt_extern.c |
