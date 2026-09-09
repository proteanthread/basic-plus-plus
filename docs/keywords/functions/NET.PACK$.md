<!--
Title:        NET.PACK$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_nil_bead.c
Generated:    no, hand-written
Status:       current
-->

# `NET.PACK$` Keyword Reference

## Source Header

```c
// FILENAME: func_nil_bead.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_bead.h, nil_bead.c, strings.h, strings.c)
// NEEDS: libengine (func_nil_bead.h)
// Implements NET.PACK$() and NIL.PACK$() RFC 51 serialization.
//
// ---- Includes ----
```

## 1. Description & Usage

Serializes variable or array into RFC 51 compact bead byte stream.

## 2. Syntax

```basic
NET.PACK$(var)
```

## 3. Code Example

```basic
10 Val = NET.PACK$(var)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Network & Protocols
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NET.PACK$ |
| Category | Network & Protocols |
| Syntax | NET.PACK$(var) |
| Description | Serializes variable or array into RFC 51 compact bead byte stream. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_nil_bead.c |
