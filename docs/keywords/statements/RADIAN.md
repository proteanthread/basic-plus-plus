<!--
Title:        RADIAN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/extended/stmt_angle.c
Generated:    no, hand-written
Status:       current
-->

# `RADIAN` Keyword Reference

## Source Header

```c
// FILENAME: stmt_angle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libkernel (vm.h, lexer.h, errors.h)
// Provides runtime implementation for DEGREE, RADIAN, and GRAD angle mode sta
//
// ---- Includes ----
```

## 1. Description & Usage

Sets trigonometric evaluation mode to radians (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).

## 2. Syntax

```basic
RADIAN
```

## 3. Code Example

```basic
10 REM RADIAN Demonstration
20 PRINT "RADIAN executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Trigonometry
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RADIAN |
| Category | Math & Trigonometry |
| Syntax | RADIAN |
| Description | Sets trigonometric evaluation mode to radians (Sharp Pocket BASIC PC-1211/PC-1500 / Casio). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/extended/stmt_angle.c |
