<!--
Title:        GOPHER.GET$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_gopher.c
Generated:    no, hand-written
Status:       current
-->

# `GOPHER.GET$` Keyword Reference

## Source Header

```c
// FILENAME: func_gopher.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, gopher.h, gopher.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements the GOPHER.GET$ built-in function for RFC 1436 Gopher requests.
//
// ---- Includes ----
```

## 1. Description & Usage

Fetches a Gopher item or menu from a gopher:// URL over RFC 1436 port 70.

## 2. Syntax

```basic
GOPHER.GET$(url$)
```

## 3. Code Example

```basic
10 Val = GOPHER.GET$(url$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Network & Cloud
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GOPHER.GET$ |
| Category | Network & Cloud |
| Syntax | GOPHER.GET$(url$) |
| Description | Fetches a Gopher item or menu from a gopher:// URL over RFC 1436 port 70. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_gopher.c |
