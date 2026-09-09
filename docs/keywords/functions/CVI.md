<!--
Title:        CVI
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/builtins/string_fn.c
Generated:    no, hand-written
Status:       current
-->

# `CVI` Keyword Reference

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

Decodes a 2-byte fielded binary string into a 16-bit signed integer.

## 2. Syntax

```basic
CVI(2byte_str$)
```

## 3. Code Example

```basic
10 Val = CVI(2byte_str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (length != 2)

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
| Name | CVI |
| Category | Binary & Byte Conversion |
| Syntax | CVI(2byte_str$) |
| Description | Decodes a 2-byte fielded binary string into a 16-bit signed integer. |
| Error Summary | Error 5: Illegal Function Call (length != 2) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/builtins/string_fn.c |
