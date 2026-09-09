<!--
Title:        REMOTE.EVAL$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_remote.c
Generated:    no, hand-written
Status:       current
-->

# `REMOTE.EVAL$` Keyword Reference

## Source Header

```c
// FILENAME: func_remote.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (nil_transport.h, nil_transport.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (func_remote.h, string.c)
// Evaluates REMOTE.EVAL$() and IOT.RPC$() functions.
//
// ---- Includes ----
```

## 1. Description & Usage

Sends an expression string to a remote node for evaluation and returns the result string.

## 2. Syntax

```basic
REMOTE.EVAL$(target$, expr$)
```

## 3. Code Example

```basic
10 Val = REMOTE.EVAL$(target$, e10pr$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13 (Type mismatch) on missing or invalid arguments

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
| Name | REMOTE.EVAL$ |
| Category | Hardware & Network |
| Syntax | REMOTE.EVAL$(target$, expr$) |
| Description | Sends an expression string to a remote node for evaluation and returns the result string. |
| Error Summary | Error 13 (Type mismatch) on missing or invalid arguments |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_remote.c |
