<!--
Title:        TEXTBOX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/textbox.c
Generated:    no, hand-written
Status:       current
-->

# `TEXTBOX` Keyword Reference

## Source Header

```c
// FILENAME: textbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (eval.h, textbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TEXTBOX statement in BASIC++.
```

## 1. Description & Usage

Renders an editable text box UI widget.

## 2. Syntax

```basic
TEXTBOX text$ [, col%, row%, width%, height%]
```

## 3. Code Example

```basic
10 REM TEXTBOX Demonstration
20 PRINT "TEXTBOX executed successfully."
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
| Name | TEXTBOX |
| Category | User Interface |
| Syntax | TEXTBOX text$ [, col%, row%, width%, height%] |
| Description | Renders an editable text box UI widget. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/textbox.c |
