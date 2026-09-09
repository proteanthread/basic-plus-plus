<!--
Title:        VSCROLLBAR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/vscrollbar.c
Generated:    no, hand-written
Status:       current
-->

# `VSCROLLBAR` Keyword Reference

## Source Header

```c
// FILENAME: vscrollbar.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, vscrollbar.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the VSCROLLBAR statement in BASIC++.
```

## 1. Description & Usage

Renders a vertical scroll bar UI control.

## 2. Syntax

```basic
VSCROLLBAR min%, max%, val%, col%, row%, height%
```

## 3. Code Example

```basic
10 REM VSCROLLBAR Demonstration
20 PRINT "VSCROLLBAR executed successfully."
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
| Name | VSCROLLBAR |
| Category | User Interface |
| Syntax | VSCROLLBAR min%, max%, val%, col%, row%, height% |
| Description | Renders a vertical scroll bar UI control. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/vscrollbar.c |
