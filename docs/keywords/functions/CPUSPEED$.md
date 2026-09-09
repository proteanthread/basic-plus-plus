<!--
Title:        CPUSPEED$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_cpuspeed.c
Generated:    no, hand-written
Status:       current
-->

# `CPUSPEED$` Keyword Reference

## Source Header

```c
// FILENAME: func_cpuspeed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Evaluator implementation for CPUSPEED and CPUSPEED$.
```

## 1. Description & Usage

Returns formatted host or target CPU speed with unit suffix (e.g. "3.40 GHz", "4.77 MHz").

## 2. Syntax

```basic
CPUSPEED$ | CPUSPEED$("model")
```

## 3. Code Example

```basic
10 Val = CPUSPEED$ | CPUSPEED$("model")
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Peripherals
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CPUSPEED$ |
| Category | Hardware & Peripherals |
| Syntax | CPUSPEED$ \| CPUSPEED$("model") |
| Description | Returns formatted host or target CPU speed with unit suffix (e.g. "3.40 GHz", "4.77 MHz"). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_cpuspeed.c |
