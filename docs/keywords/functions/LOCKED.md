<!--
Title:        LOCKED
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/func_record_lock.c
Generated:    no, hand-written
Status:       current
-->

# `LOCKED` Keyword Reference

## Source Header

```c
// FILENAME: func_record_lock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dispatch_call.c)
// NEEDS: libcore (language_descriptor.h, strings.h), libengine (func_record_l
// Provides runtime function implementation for READU, WRITEU, RELEASE, and LO
//
// ---- Includes ----
```

## 1. Description & Usage

Returns -1 if record is locked, 0 if unlocked.

## 2. Syntax

```basic
is_locked = LOCKED(ch, id) | LOCKED[ch, id]
```

## 3. Code Example

```basic
10 Val = is_locked = LOCKED(ch, id) | LOCKED[ch, id]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Pick & Business BASIC
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOCKED |
| Category | Pick & Business BASIC |
| Syntax | is_locked = LOCKED(ch, id) \| LOCKED[ch, id] |
| Description | Returns -1 if record is locked, 0 if unlocked. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/func_record_lock.c |
