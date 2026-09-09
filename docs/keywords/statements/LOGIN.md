<!--
Title:        LOGIN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/login.c
Generated:    no, hand-written
Status:       current
-->

# `LOGIN` Keyword Reference

## Source Header

```c
// FILENAME: login.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h, strings.h)
// NEEDS: libengine (eval.h, login.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the LOGIN / HELLO statement in BASIC++.
```

## 1. Description & Usage

Logs in a timesharing user session with optional account code.

## 2. Syntax

```basic
LOGIN username$ [, account$] | HELLO username$ [, account$]
```

## 3. Code Example

```basic
10 REM LOGIN Demonstration
20 PRINT "LOGIN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Session & Multi-User
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOGIN |
| Category | Session & Multi-User |
| Syntax | LOGIN username$ [, account$] \| HELLO username$ [, account$] |
| Description | Logs in a timesharing user session with optional account code. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/login.c |
