<!--
Title:        MKISTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/builtins/string_fn.c
Generated:    no, hand-written
Status:       current
-->

# `MKISTR` Keyword Reference

## Source Header

```c
// FILENAME: string_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Provides core logic and interface definitions for string_fn within BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Encodes a 16-bit signed integer into a 2-byte fielded binary string.

## 2. Syntax

```basic
MKISTR(int_val%) | MKI$(int_val%)
```

## 3. Code Example

```basic
10 Val = MKISTR(int_val%) | MKI$(int_val%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 14: Out of String Space

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Binary & Byte Conversion
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MKISTR |
| Category | Binary & Byte Conversion |
| Syntax | MKISTR(int_val%) \| MKI$(int_val%) |
| Description | Encodes a 16-bit signed integer into a 2-byte fielded binary string. |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/builtins/string_fn.c |
