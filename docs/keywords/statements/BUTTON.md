<!--
Title:        BUTTON
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/button.c
Generated:    no, hand-written
Status:       current
-->

# `BUTTON` Keyword Reference

## Source Header

```c
// FILENAME: button.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (button.h, eval.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the BUTTON statement in BASIC++.
```

## 1. Description & Usage

Renders a command button UI widget with caption.

## 2. Syntax

```basic
BUTTON caption$ [, col%, row%, width%, height%]
```

## 3. Code Example

```basic
10 REM BUTTON Demonstration
20 PRINT "BUTTON executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: User Interface
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BUTTON |
| Category | User Interface |
| Syntax | BUTTON caption$ [, col%, row%, width%, height%] |
| Description | Renders a command button UI widget with caption. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/button.c |
