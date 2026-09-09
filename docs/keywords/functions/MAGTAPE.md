<!--
Title:        MAGTAPE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/magtape.c
Generated:    no, hand-written
Status:       current
-->

# `MAGTAPE` Keyword Reference

## Source Header

```c
// FILENAME: magtape.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (magtape.h, string.c)
// Provides runtime implementation for the MAGTAPE built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Performs magnetic tape operations (rewind, space, write EOF) on a file channel (DEC RSTS/E).

## 2. Syntax

```basic
MAGTAPE(function_code, channel_num, arg)
```

## 3. Code Example

```basic
10 Val = MAGTAPE(function_code, channel_num, arg)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MAGTAPE |
| Category | System Functions |
| Syntax | MAGTAPE(function_code, channel_num, arg) |
| Description | Performs magnetic tape operations (rewind, space, write EOF) on a file channel (DEC RSTS/E). |
| Error Summary | Error 13: Type Mismatch, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/magtape.c |
