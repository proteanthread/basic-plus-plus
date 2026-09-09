<!--
Title:        OPTIONBUTTON
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/optionbutton.c
Generated:    no, hand-written
Status:       current
-->

# `OPTIONBUTTON` Keyword Reference

## Source Header

```c
// FILENAME: optionbutton.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, optionbutton.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the OPTIONBUTTON statement in BASIC++.
```

## 1. Description & Usage

Renders a radio/option button UI control.

## 2. Syntax

```basic
OPTIONBUTTON caption$ [, selected% [, col%, row%, width%]]
```

## 3. Code Example

```basic
10 REM OPTIONBUTTON Demonstration
20 PRINT "OPTIONBUTTON executed successfully."
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
| Name | OPTIONBUTTON |
| Category | User Interface |
| Syntax | OPTIONBUTTON caption$ [, selected% [, col%, row%, width%]] |
| Description | Renders a radio/option button UI control. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/optionbutton.c |
