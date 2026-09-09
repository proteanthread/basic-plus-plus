<!--
Title:        ASK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/ask.c
Generated:    no, hand-written
Status:       current
-->

# `ASK` Keyword Reference

## Source Header

```c
// FILENAME: ask.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h, variables.h, variables.c)
// NEEDS: libengine (ask.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the ASK statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to query console, graphics, and environment attributes.

## 2. Syntax

```basic
ASK property variable [, ...]
```

## 3. Code Example

```basic
10 REM ASK Demonstration
20 PRINT "ASK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Environment & Graphics
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ASK |
| Category | Environment & Graphics |
| Syntax | ASK property variable [, ...] |
| Description | ECMA-116 standard statement to query console, graphics, and environment attributes. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/ask.c |
