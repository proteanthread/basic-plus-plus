<!--
Title:        GEMINI.STATUS%
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_gemini_meta.c
Generated:    no, hand-written
Status:       current
-->

# `GEMINI.STATUS%` Keyword Reference

## Source Header

```c
// FILENAME: func_gemini_meta.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (gemini.h, gemini.c, string.h, strings.h, strings.c)
// NEEDS: libengine (func_gemini_meta.h, string.c)
// Implements GEMINI.STATUS%() and GEMINI.META$() built-in functions.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the integer status code of the last Gemini protocol request.

## 2. Syntax

```basic
GEMINI.STATUS%()
```

## 3. Code Example

```basic
10 Val = GEMINI.STATUS%()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Network & Protocols
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GEMINI.STATUS% |
| Category | Network & Protocols |
| Syntax | GEMINI.STATUS%() |
| Description | Returns the integer status code of the last Gemini protocol request. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_gemini_meta.c |
