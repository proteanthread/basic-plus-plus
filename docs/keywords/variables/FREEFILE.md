<!--
Title:        FREEFILE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/descriptors/system_var_descriptors.c
Generated:    no, hand-written
Status:       current
-->

# `FREEFILE` Keyword Reference

## Source Header

```c
// FILENAME: system_var_descriptors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (language_descriptor.h, system_var_descriptors.h)
// Implements authoritative descriptor definitions for built-in system variabl
```

## 1. Description & Usage

Returns the next available unused file channel number (#1..#255).

## 2. Syntax

```basic
FREEFILE or FREEFILE()
```

## 3. Code Example

```basic
10 Val = FREEFILE or FREEFILE()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Diagnostics
- **Subsystem**: SUBSYSTEM_SCRIPT
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FREEFILE |
| Category | Diagnostics |
| Syntax | FREEFILE or FREEFILE() |
| Description | Returns the next available unused file channel number (#1..#255). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SCRIPT |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/runtime/descriptors/system_var_descriptors.c |
