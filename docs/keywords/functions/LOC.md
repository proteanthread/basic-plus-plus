<!--
Title:        LOC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/status/loc_fn.c
Generated:    no, hand-written
Status:       current
-->

# `LOC` Keyword Reference

## Source Header

```c
// FILENAME: loc_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libengine (loc_fn.h, vm.h)
// Provides runtime implementation for the LOC_FN built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the current read/write position in an open file.

## 2. Syntax

```basic
pos& = LOC(file_num%)
```

## 3. Code Example

```basic
10 Val = pos& = LOC(file_num%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (LOC expects numeric channel)

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
| Name | LOC |
| Category | Filesystem Functions |
| Syntax | pos& = LOC(file_num%) |
| Description | Returns the current read/write position in an open file. |
| Error Summary | Error 13: Type Mismatch (LOC expects numeric channel) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/status/loc_fn.c |
