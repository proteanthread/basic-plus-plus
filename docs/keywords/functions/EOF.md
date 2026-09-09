<!--
Title:        EOF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/status/eof_fn.c
Generated:    no, hand-written
Status:       current
-->

# `EOF` Keyword Reference

## Source Header

```c
// FILENAME: eof_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libengine (eof_fn.h, vm.h)
// Provides runtime implementation for the EOF_FN built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns true (-1) if end-of-file is reached on the specified channel, otherwise false (0).

## 2. Syntax

```basic
flag% = EOF(file_num%)
```

## 3. Code Example

```basic
10 Val = flag% = EOF(file_num%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (EOF expects numeric channel)

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
| Name | EOF |
| Category | Filesystem Functions |
| Syntax | flag% = EOF(file_num%) |
| Description | Returns true (-1) if end-of-file is reached on the specified channel, otherwise false (0). |
| Error Summary | Error 13: Type Mismatch (EOF expects numeric channel) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/status/eof_fn.c |
