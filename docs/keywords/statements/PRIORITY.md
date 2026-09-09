<!--
Title:        PRIORITY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/priority.c
Generated:    no, hand-written
Status:       current
-->

# `PRIORITY` Keyword Reference

## Source Header

```c
// FILENAME: priority.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, priority.h)
// Provides runtime implementation for the PRIORITY statement in BASIC++.
```

## 1. Description & Usage

Sets timesharing CPU job execution scheduling priority (1..100).

## 2. Syntax

```basic
PRIORITY priority_level%
```

## 3. Code Example

```basic
10 REM PRIORITY Demonstration
20 PRINT "PRIORITY executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | PRIORITY |
| Category | Session & Multi-User |
| Syntax | PRIORITY priority_level% |
| Description | Sets timesharing CPU job execution scheduling priority (1..100). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/priority.c |
