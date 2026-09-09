<!--
Title:        SCALE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/external/scale.c
Generated:    no, hand-written
Status:       current
-->

# `SCALE` Keyword Reference

## Source Header

```c
// FILENAME: scale.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, scale.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SCALE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets fixed-point decimal scale factor (0 to 6) for arithmetic precision and rounding.

## 2. Syntax

```basic
SCALE factor
```

## 3. Code Example

```basic
10 REM SCALE Demonstration
20 PRINT "SCALE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Numeric & Math
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SCALE |
| Category | Numeric & Math |
| Syntax | SCALE factor |
| Description | Sets fixed-point decimal scale factor (0 to 6) for arithmetic precision and rounding. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/external/scale.c |
