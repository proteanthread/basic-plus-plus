<!--
Title:        GET
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/file_ops/get.c
Generated:    no, hand-written
Status:       current
-->

# `GET` Keyword Reference

## Source Header

```c
// FILENAME: get.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (bgi.h, bgi.c, eval.h, eval.c, get.h, lexer.h, lexer.c)
// NEEDS: libengine (map.h, map.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GET statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads a record from a random-access file or captures a screen rectangle into a memory array.

## 2. Syntax

```basic
GET [#]file_num [, record_number] | GET (x1, y1)-(x2, y2), array_name
```

## 3. Code Example

```basic
10 Val = GET [#]file_num [, record_number] | GET (101, y1)-(102, y2), array_name
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem I/O & Graphics
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GET |
| Category | Filesystem I/O & Graphics |
| Syntax | GET [#]file_num [, record_number] \| GET (x1, y1)-(x2, y2), array_name |
| Description | Reads a record from a random-access file or captures a screen rectangle into a memory array. |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/file_ops/get.c |
