<!--
Title:        UPNP.STATUS$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/network/func_upnp.c
Generated:    no, hand-written
Status:       current
-->

# `UPNP.STATUS$` Keyword Reference

## Source Header

```c
// FILENAME: func_upnp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (builtins, sys_fn.c, string_fn.c)
// NEEDS: libcore (memory.h, strings.h), libserver (vnet_nat.h)
// Implements UPnP functions (UPNP.EXTERNALIP$, UPNP.STATUS, UPNP.STATUS$).
```

## 1. Description & Usage

Returns descriptive status string of the specified UPnP port forwarding mapping.

## 2. Syntax

```basic
UPNP.STATUS$(port[, proto$])
```

## 3. Code Example

```basic
10 Val = UPNP.STATUS$(port[, proto$])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Network & UPnP
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UPNP.STATUS$ |
| Category | Network & UPnP |
| Syntax | UPNP.STATUS$(port[, proto$]) |
| Description | Returns descriptive status string of the specified UPnP port forwarding mapping. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/network/func_upnp.c |
