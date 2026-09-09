<!--
Title:        INPUTBOX$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/ui/dialogs/inputbox.c
Generated:    no, hand-written
Status:       current
-->

# `INPUTBOX$` Keyword Reference

## Source Header

```c
// FILENAME: inputbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (inputbox.h)
// Provides runtime implementation for the INPUTBOX$ function in BASIC++.
```

## 1. Description & Usage

Displays a prompt in a dialog box and returns the text entered by the user.

## 2. Syntax

```basic
res$ = INPUTBOX$(prompt$ [, title$ [, default$]])
```

## 3. Code Example

```basic
10 Val = res$ = INPUTBOX$(prompt$ [, title$ [, default$]])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

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
| Name | INPUTBOX$ |
| Category | User Interface |
| Syntax | res$ = INPUTBOX$(prompt$ [, title$ [, default$]]) |
| Description | Displays a prompt in a dialog box and returns the text entered by the user. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/ui/dialogs/inputbox.c |
