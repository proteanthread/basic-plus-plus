<!--
Title:        DRIVELISTBOX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/drivelistbox.c
Generated:    no, hand-written
Status:       current
-->

# `DRIVELISTBOX` Keyword Reference

## Source Header

```c
// FILENAME: drivelistbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, drivelistbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the DRIVELISTBOX statement in BASIC++.
```

## 1. Description & Usage

Renders a drive list box UI control for disk volume selection.

## 2. Syntax

```basic
DRIVELISTBOX col%, row%, width%
```

## 3. Code Example

```basic
10 REM DRIVELISTBOX Demonstration
20 PRINT "DRIVELISTBOX executed successfully."
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
| Name | DRIVELISTBOX |
| Category | User Interface |
| Syntax | DRIVELISTBOX col%, row%, width% |
| Description | Renders a drive list box UI control for disk volume selection. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/drivelistbox.c |
