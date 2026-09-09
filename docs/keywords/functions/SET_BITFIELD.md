<!--
Title:        SET_BITFIELD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/func_systems_types.c
Generated:    no, hand-written
Status:       current
-->

# `SET_BITFIELD` Keyword Reference

## Source Header

```c
// FILENAME: func_systems_types.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h), libengine (func_systems_types.h)
// Provides runtime function implementation for systems types, pointers, and m
//
// ---- Includes ----
```

## 1. Description & Usage

Sets bitfield of length bit_count at start_bit to new_val.

## 2. Syntax

```basic
res = SET_BITFIELD(val, start_bit, bit_count, new_val)
```

## 3. Code Example

```basic
10 Val = res = SET_BITFIELD(val, start_bit, bit_count, new_val)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Systems Types
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SET_BITFIELD |
| Category | Systems Types |
| Syntax | res = SET_BITFIELD(val, start_bit, bit_count, new_val) |
| Description | Sets bitfield of length bit_count at start_bit to new_val. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/func_systems_types.c |
