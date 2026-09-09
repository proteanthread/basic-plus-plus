<!--
Title:        MSGBOX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/msgbox.c
Generated:    no, hand-written
Status:       current
-->

# `MSGBOX` Keyword Reference

## Source Header

```c
// FILENAME: msgbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, msgbox.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the MSGBOX statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Displays a modal message box dialog on the active console or terminal.

## 2. Syntax

```basic
MSGBOX prompt$ [, [buttons%] [, title$]]
```

## 3. Code Example

```basic
10 REM MSGBOX Demonstration
20 PRINT "MSGBOX executed successfully."
```

## 4. Error Conditions

Error 5: Invalid context, Error 13: Type mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: User Interface
- **Subsystem**: SUBSYSTEM_STANDARD
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MSGBOX |
| Category | User Interface |
| Syntax | MSGBOX prompt$ [, [buttons%] [, title$]] |
| Description | Displays a modal message box dialog on the active console or terminal. |
| Error Summary | Error 5: Invalid context, Error 13: Type mismatch |
| Subsystem | SUBSYSTEM_STANDARD |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/msgbox.c |
