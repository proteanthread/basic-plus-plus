<!--
Title:        SUB
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/sub.c
Generated:    no, hand-written
Status:       current
-->

# `SUB` Keyword Reference

## Source Header

```c
// FILENAME: sub.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, eval_expr_internal.h, exec_control_internal.h
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, let.c, ops.c)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libengine (sub_internal.h)
// Provides runtime implementation for the SUB statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares a named subroutine block with formal parameters.

## 2. Syntax

```basic
SUB name [(param1, param2...)] [STATIC]
```

## 3. Code Example

```basic
10 Val = SUB name [(param1, param2...)] [STATIC]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 35: Undefined SUB

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Procedures & OOP
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SUB |
| Category | Procedures & OOP |
| Syntax | SUB name [(param1, param2...)] [STATIC] |
| Description | Declares a named subroutine block with formal parameters. |
| Error Summary | Error 2: Syntax Error, Error 35: Undefined SUB |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/sub.c |
