<!--
Title:        IOT.RPC$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_remote.c
Generated:    no, hand-written
Status:       current
-->

# `IOT.RPC$` Keyword Reference

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

Executes a remote procedure call on an IoT target node and returns the response string.

## 2. Syntax

```basic
IOT.RPC$(target$, func_call$)
```

## 3. Code Example

```basic
10 Val = IOT.RPC$(target$, func_call$)
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
| Name | IOT.RPC$ |
| Category | Hardware & Network |
| Syntax | IOT.RPC$(target$, func_call$) |
| Description | Executes a remote procedure call on an IoT target node and returns the response string. |
| Error Summary | Error 13 (Type mismatch) on missing or invalid arguments |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_remote.c |
