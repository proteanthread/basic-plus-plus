<!--
Title:        DECLARE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/external/declare.c
Generated:    no, hand-written
Status:       current
-->

# `DECLARE` Keyword Reference

## Source Header

```c
// FILENAME: declare.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (declare.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DECLARE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares a SUB or FUNCTION procedure prototype in QuickBASIC and ECMA-116 standard BASIC.

## 2. Syntax

```basic
DECLARE {SUB | FUNCTION} name [ALIAS "aliasname"] [(params)]
```

## 3. Code Example

```basic
10 Val = DECLARE {SUB | FUNCTION} name [ALIAS "aliasname"] [(params)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error

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
| Name | DECLARE |
| Category | Control Flow |
| Syntax | DECLARE {SUB \| FUNCTION} name [ALIAS "aliasname"] [(params)] |
| Description | Declares a SUB or FUNCTION procedure prototype in QuickBASIC and ECMA-116 standard BASIC. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/external/declare.c |
