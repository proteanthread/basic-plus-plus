<!--
Title:        MENU
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/menu.c
Generated:    no, hand-written
Status:       current
-->

# `MENU` Keyword Reference

## Source Header

```c
// FILENAME: menu.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (eval.h, menu.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the MENU statement in BASIC++.
```

## 1. Description & Usage

Renders a menu item UI widget or menu bar header.

## 2. Syntax

```basic
MENU [menu_title$,] item_caption$
```

## 3. Code Example

```basic
10 REM MENU Demonstration
20 PRINT "MENU executed successfully."
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
| Name | MENU |
| Category | User Interface |
| Syntax | MENU [menu_title$,] item_caption$ |
| Description | Renders a menu item UI widget or menu bar header. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/menu.c |
