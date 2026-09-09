<!--
Title:        BITFIELD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/bits/manipulation/bitfield.c
Generated:    no, hand-written
Status:       current
-->

# `BITFIELD` Keyword Reference

## Source Header

```c
// FILENAME: bitfield.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (bitfield.h)
// Provides runtime implementation for the BITFIELD built-in function in BASIC
```

## 1. Description & Usage

Extracts a bitfield of length len starting at zero-based bit index start.

## 2. Syntax

```basic
BITFIELD(val, start, len) or BITFIELD[val, start, len]
```

## 3. Code Example

```basic
10 Val = BITFIELD(val, start, len) or BITFIELD[val, start, len]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Bitwise & Logical Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BITFIELD |
| Category | Bitwise & Logical Functions |
| Syntax | BITFIELD(val, start, len) or BITFIELD[val, start, len] |
| Description | Extracts a bitfield of length len starting at zero-based bit index start. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/bits/manipulation/bitfield.c |
