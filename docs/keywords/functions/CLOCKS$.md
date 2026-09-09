<!--
Title:        CLOCKS$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_clocks.c
Generated:    no, hand-written
Status:       current
-->

# `CLOCKS$` Keyword Reference

## Source Header

```c
// FILENAME: func_clocks.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Evaluator implementation for CLOCKS and CLOCKS$.
```

## 1. Description & Usage

Returns multi-domain hardware clock summary string (CPU, Bus, Memory).

## 2. Syntax

```basic
CLOCKS$
```

## 3. Code Example

```basic
10 REM CLOCKS$ Demonstration
20 PRINT "CLOCKS$ executed successfully."
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
| Name | CLOCKS$ |
| Category | Hardware & Peripherals |
| Syntax | CLOCKS$ |
| Description | Returns multi-domain hardware clock summary string (CPU, Bus, Memory). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_clocks.c |
