<!--
Title:        WHO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/who.c
Generated:    no, hand-written
Status:       current
-->

# `WHO` Keyword Reference

## Source Header

```c
// FILENAME: who.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, who.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the WHO statement in BASIC++.
```

## 1. Description & Usage

Displays active user session telemetry, job ID, TTY line, and priority status.

## 2. Syntax

```basic
WHO
```

## 3. Code Example

```basic
10 REM WHO Demonstration
20 PRINT "WHO executed successfully."
```

## 4. Error Conditions

None

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
| Name | WHO |
| Category | Session & Multi-User |
| Syntax | WHO |
| Description | Displays active user session telemetry, job ID, TTY line, and priority status. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/who.c |
