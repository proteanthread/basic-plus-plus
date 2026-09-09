<!--
Title:        TYP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/descriptors/typ.c
Generated:    no, hand-written
Status:       current
-->

# `TYP` Keyword Reference

## Source Header

```c
// FILENAME: typ.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (file.h, file.c, funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (typ.h, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the TYP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Apple /// Business BASIC channel record type introspection (0=EOF, 1=Integer, 2=Real, 3=String).

## 2. Syntax

```basic
type_code% = TYP(#channel) | TYP(channel)
```

## 3. Code Example

```basic
10 Val = type_code% = TYP(#channel) | TYP(channel)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TYP |
| Category | Filesystem |
| Syntax | type_code% = TYP(#channel) \| TYP(channel) |
| Description | Apple /// Business BASIC channel record type introspection (0=EOF, 1=Integer, 2=Real, 3=String). |
| Error Summary | Error 13: Type Mismatch, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/descriptors/typ.c |
