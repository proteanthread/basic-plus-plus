<!--
Title:        VIEWPORT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/bgi/viewport.c
Generated:    no, hand-written
Status:       current
-->

# `VIEWPORT` Keyword Reference

## Source Header

```c
// FILENAME: viewport.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, viewport.h)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the VIEWPORT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

ECMA-116 standard statement to specify a normalized graphics viewport boundary.

## 2. Syntax

```basic
VIEWPORT xmin, xmax, ymin, ymax
```

## 3. Code Example

```basic
10 REM VIEWPORT Demonstration
20 PRINT "VIEWPORT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Sound
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VIEWPORT |
| Category | Graphics & Sound |
| Syntax | VIEWPORT xmin, xmax, ymin, ymax |
| Description | ECMA-116 standard statement to specify a normalized graphics viewport boundary. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/bgi/viewport.c |
