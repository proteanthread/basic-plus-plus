<!--
Title:        KEYCOUNT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/func_isam_fn.c
Generated:    no, hand-written
Status:       current
-->

# `KEYCOUNT` Keyword Reference

## Source Header

```c
// FILENAME: func_isam_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h, strings.h), libengine (func_isam_fn.
// Provides runtime implementation for KEY$, KEYCOUNT, and ISAM functions in B
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the number of keys in the active index of an ISAM channel.

## 2. Syntax

```basic
cnt = KEYCOUNT(ch) | KEYCOUNT[ch]
```

## 3. Code Example

```basic
10 Val = cnt = KEYCOUNT(ch) | KEYCOUNT[ch]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Database
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | KEYCOUNT |
| Category | Database |
| Syntax | cnt = KEYCOUNT(ch) \| KEYCOUNT[ch] |
| Description | Returns the number of keys in the active index of an ISAM channel. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/func_isam_fn.c |
