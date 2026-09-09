<!--
Title:        COMSTR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/func_nil_compress.c
Generated:    no, hand-written
Status:       current
-->

# `COMSTR$` Keyword Reference

## Source Header

```c
// FILENAME: func_nil_compress.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_compress.h, nil_compress.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (func_nil_compress.h, string.c)
// Evaluates COMSTR$(), DECOMSTR$(), NIL.COMPRESS$(), NIL.DECOMPRESS$() built-
//
// ---- Includes ----
```

## 1. Description & Usage

Compresses input string using RFC 51 Run-Length Encoding and byte packing.

## 2. Syntax

```basic
COMSTR$(str$)
```

## 3. Code Example

```basic
10 Val = COMSTR$(str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (string argument required)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Manipulation
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | COMSTR$ |
| Category | String Manipulation |
| Syntax | COMSTR$(str$) |
| Description | Compresses input string using RFC 51 Run-Length Encoding and byte packing. |
| Error Summary | Error 13: Type Mismatch (string argument required) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/func_nil_compress.c |
