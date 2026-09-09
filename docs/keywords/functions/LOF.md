<!--
Title:        LOF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/status/lof.c
Generated:    no, hand-written
Status:       current
-->

# `LOF` Keyword Reference

## Source Header

```c
// FILENAME: lof.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libengine (lof.h, vm.h)
// Provides runtime implementation for the LOF built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the length of an open file in bytes.

## 2. Syntax

```basic
length& = LOF(file_num%)
```

## 3. Code Example

```basic
10 Val = length& = LOF(file_num%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (LOF expects numeric channel)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOF |
| Category | Filesystem Functions |
| Syntax | length& = LOF(file_num%) |
| Description | Returns the length of an open file in bytes. |
| Error Summary | Error 13: Type Mismatch (LOF expects numeric channel) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/status/lof.c |
