<!--
Title:        FUJI.STATUS$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_fujinet.c
Generated:    no, hand-written
Status:       current
-->

# `FUJI.STATUS$` Keyword Reference

## Source Header

```c
// FILENAME: func_fujinet.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (func_fujinet.h, string.c)
// Implements FUJI.STATUS$(), FUJI.SSID$(), FUJI.IP$(), and FUJI.JSON.GET$() f
//
// ---- Includes ----
```

## 1. Description & Usage

Returns active FujiNet adapter network connection status string.

## 2. Syntax

```basic
FUJI.STATUS$()
```

## 3. Code Example

```basic
10 Val = FUJI.STATUS$()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & FujiNet
- **Subsystem**: SUBSYSTEM_HARDWARE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FUJI.STATUS$ |
| Category | Hardware & FujiNet |
| Syntax | FUJI.STATUS$() |
| Description | Returns active FujiNet adapter network connection status string. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_fujinet.c |
