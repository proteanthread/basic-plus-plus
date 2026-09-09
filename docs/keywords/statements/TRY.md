<!--
Title:        TRY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/try.c
Generated:    no, hand-written
Status:       current
-->

# `TRY` Keyword Reference

## Source Header

```c
// FILENAME: try.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, dialect.h, dialect.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (config.h)
// Provides runtime implementation for the TRY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Structured exception-handling block supporting TRY, CATCH, FINALLY, and END TRY.

## 2. Syntax

```basic
TRY ... CATCH err_var ... FINALLY ... END TRY
```

## 3. Code Example

```basic
10 REM TRY Demonstration
20 PRINT "TRY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 35: TRY Without CATCH/FINALLY

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TRY |
| Category | Event Trapping |
| Syntax | TRY ... CATCH err_var ... FINALLY ... END TRY |
| Description | Structured exception-handling block supporting TRY, CATCH, FINALLY, and END TRY. |
| Error Summary | Error 2: Syntax Error, Error 35: TRY Without CATCH/FINALLY |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/try.c |
