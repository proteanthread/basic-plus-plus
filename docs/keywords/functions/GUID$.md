<!--
Title:        GUID$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/eval.c
Generated:    no, hand-written
Status:       current
-->

# `GUID$` Keyword Reference

## Source Header

```c
// FILENAME: eval.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext, libkernel, libstandard
// NEEDS: libengine (eval_expr_internal.h)
// Implements component functionality for eval.c.
//
// ---- Includes ----
```

## 1. Description & Usage

Generates a canonical RFC 4122 formatted Version 4 UUID string (e.g. 'c4a760a8-dbcf-5254-a0d9-6a4474dbb14e').

## 2. Syntax

```basic
GUID$
```

## 3. Code Example

```basic
10 REM GUID$ Demonstration
20 PRINT "GUID$ executed successfully."
```

## 4. Error Conditions

Error 14: Out of String Space

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & OS
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GUID$ |
| Category | System & OS |
| Syntax | GUID$ |
| Description | Generates a canonical RFC 4122 formatted Version 4 UUID string (e.g. 'c4a760a8-dbcf-5254-a0d9-6a4474dbb14e'). |
| Error Summary | Error 14: Out of String Space |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/eval.c |
