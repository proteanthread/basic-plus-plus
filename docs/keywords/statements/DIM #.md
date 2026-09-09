<!--
Title:        DIM #
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/vdim.c
Generated:    no, hand-written
Status:       current
-->

# `DIM #` Keyword Reference

## Source Header

```c
// FILENAME: vdim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (dim.h, dim.c, vdim.h)
// Provides runtime implementation for the VDIM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Allocates a DEC BASIC-PLUS virtual array mapped to an open random or sequential file channel.

## 2. Syntax

```basic
DIM #channel, array_name(bounds)
```

## 3. Code Example

```basic
10 DIM Arr(10)
20 Arr(5) = 100
30 PRINT "Arr(5) = "; Arr(5)
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 10: Array Already Dimensioned, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File System & Virtual Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DIM # |
| Category | File System & Virtual Memory |
| Syntax | DIM #channel, array_name(bounds) |
| Description | Allocates a DEC BASIC-PLUS virtual array mapped to an open random or sequential file channel. |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 10: Array Already Dimensioned, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/vdim.c |
