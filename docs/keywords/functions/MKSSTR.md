<!--
Title:        MKSSTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/builtins/string_fn.c
Generated:    no, hand-written
Status:       current
-->

# `MKSSTR` Keyword Reference

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

Encodes a single-precision floating-point number into a 4-byte fielded binary string.

## 2. Syntax

```basic
MKSSTR(sng_val!) | MKS$(sng_val!)
```

## 3. Code Example

```basic
10 Val = MKSSTR(sng_val!) | MKS$(sng_val!)
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
| Name | MKSSTR |
| Category | Binary & Byte Conversion |
| Syntax | MKSSTR(sng_val!) \| MKS$(sng_val!) |
| Description | Encodes a single-precision floating-point number into a 4-byte fielded binary string. |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/builtins/string_fn.c |
