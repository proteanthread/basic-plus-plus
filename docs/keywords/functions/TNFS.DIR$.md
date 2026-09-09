<!--
Title:        TNFS.DIR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_tnfs.c
Generated:    no, hand-written
Status:       current
-->

# `TNFS.DIR$` Keyword Reference

## Source Header

```c
// FILENAME: func_tnfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (string.h, strings.h, strings.c, tnfs.h, tnfs.c)
// NEEDS: libengine (func_tnfs.h, string.c)
// Implements TNFS.DIR$ built-in function.
//
// ---- Includes ----
```

## 1. Description & Usage

Retrieves directory listing from a mounted TNFS network file system.

## 2. Syntax

```basic
TNFS.DIR$([path$ [, pattern$]])
```

## 3. Code Example

```basic
10 Val = TNFS.DIR$([path$ [, pattern$]])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Network
- **Subsystem**: SUBSYSTEM_HARDWARE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TNFS.DIR$ |
| Category | Hardware & Network |
| Syntax | TNFS.DIR$([path$ [, pattern$]]) |
| Description | Retrieves directory listing from a mounted TNFS network file system. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_tnfs.c |
