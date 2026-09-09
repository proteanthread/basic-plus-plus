<!--
Title:        TRANSLATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/extended/stmt_translate.c
Generated:    no, hand-written
Status:       current
-->

# `TRANSLATE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_translate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h, translate.h)
// Provides runtime implementation for the TRANSLATE statement (Wang 2200).
//
// ---- Includes ----
```

## 1. Description & Usage

Performs in-place character translation on a string variable (Wang 2200).

## 2. Syntax

```basic
TRANSLATE var$ USING table$ | TRANSLATE var$, from$, to$ | TRANSLATE[from$, to$] var$
```

## 3. Code Example

```basic
10 REM TRANSLATE Demonstration
20 PRINT "TRANSLATE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Strings & Formatting
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TRANSLATE |
| Category | Strings & Formatting |
| Syntax | TRANSLATE var$ USING table$ \| TRANSLATE var$, from$, to$ \| TRANSLATE[from$, to$] var$ |
| Description | Performs in-place character translation on a string variable (Wang 2200). |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/extended/stmt_translate.c |
