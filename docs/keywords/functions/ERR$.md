<!--
Title:        ERR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/error/error.c
Generated:    no, hand-written
Status:       current
-->

# `ERR$` Keyword Reference

## Source Header

```c
// FILENAME: error.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for ERR$.
```

## 1. Description & Usage

Returns the authoritative textual error description string for the last error or specified code.

## 2. Syntax

```basic
ERR$ [(error_code%)]
```

## 3. Code Example

```basic
10 Val = ERR$ [(error_code%)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Error Handling
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ERR$ |
| Category | Error Handling |
| Syntax | ERR$ [(error_code%)] |
| Description | Returns the authoritative textual error description string for the last error or specified code. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/error/error.c |
