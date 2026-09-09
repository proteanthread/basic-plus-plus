<!--
Title:        LABEL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/label.c
Generated:    no, hand-written
Status:       current
-->

# `LABEL` Keyword Reference

## Source Header

```c
// FILENAME: label.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, label.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the LABEL statement in BASIC++.
```

## 1. Description & Usage

Renders a static text label UI control.

## 2. Syntax

```basic
LABEL text$ [, col%, row%, width%]
```

## 3. Code Example

```basic
10 REM LABEL Demonstration
20 PRINT "LABEL executed successfully."
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
| Name | LABEL |
| Category | User Interface |
| Syntax | LABEL text$ [, col%, row%, width%] |
| Description | Renders a static text label UI control. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/label.c |
