<!--
Title:        WHENEVER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/whenever.c
Generated:    no, hand-written
Status:       current
-->

# `WHENEVER` Keyword Reference

## Source Header

```c
// FILENAME: whenever.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, whenever.h)
// Provides runtime implementation for the WHENEVER statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Establishes conditional event traps and exception handlers (IBM VS BASIC / CMS).

## 2. Syntax

```basic
WHENEVER {ERROR | [NOT] EOF #channel} THEN {GOTO line | statement}
```

## 3. Code Example

```basic
10 REM WHENEVER Demonstration
20 PRINT "WHENEVER executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 52: Bad File Number

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
| Name | WHENEVER |
| Category | Event Trapping |
| Syntax | WHENEVER {ERROR \| [NOT] EOF #channel} THEN {GOTO line \| statement} |
| Description | Establishes conditional event traps and exception handlers (IBM VS BASIC / CMS). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/whenever.c |
