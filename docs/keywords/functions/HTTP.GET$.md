<!--
Title:        HTTP.GET$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_http.c
Generated:    no, hand-written
Status:       current
-->

# `HTTP.GET$` Keyword Reference

## Source Header

```c
// FILENAME: func_http.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, vm.h)
// NEEDS: libserver (iot_net.h, iot_net.c)
// Implements the HTTP.GET$ built-in function for REST web client requests.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs an HTTP GET request to a remote web server and returns response body.

## 2. Syntax

```basic
HTTP.GET$(url$)
```

## 3. Code Example

```basic
10 Val = HTTP.GET$(url$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Wireless & IoT
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | HTTP.GET$ |
| Category | Wireless & IoT |
| Syntax | HTTP.GET$(url$) |
| Description | Performs an HTTP GET request to a remote web server and returns response body. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_http.c |
