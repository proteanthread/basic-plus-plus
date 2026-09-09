<!--
Title:        FILESAVEBOX$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/ui/dialogs/filesavebox.c
Generated:    no, hand-written
Status:       current
-->

# `FILESAVEBOX$` Keyword Reference

## Source Header

```c
// FILENAME: filesavebox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (filesavebox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the FILESAVEBOX$ function in BASIC++.
```

## 1. Description & Usage

Displays a file save dialog and returns the chosen filename.

## 2. Syntax

```basic
f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]])
```

## 3. Code Example

```basic
10 Val = f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]])
20 PRINT "Result: "; Val
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
| Name | FILESAVEBOX$ |
| Category | User Interface |
| Syntax | f$ = FILESAVEBOX$([pattern$ [, title$ [, default$]]]) |
| Description | Displays a file save dialog and returns the chosen filename. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/ui/dialogs/filesavebox.c |
