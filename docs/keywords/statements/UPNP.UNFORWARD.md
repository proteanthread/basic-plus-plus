<!--
Title:        UPNP.UNFORWARD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_upnp.c
Generated:    no, hand-written
Status:       current
-->

# `UPNP.UNFORWARD` Keyword Reference

## Source Header

```c
// FILENAME: stmt_upnp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libserver (vnet_nat.h, stmt_upnp.h)
// NEEDS: libengine (eval.h, lexer.h, vm.h), libcore (memory.h, strings.h)
// Implements UPnP IGD and NAT-PMP port forwarding statements (UPNP.FORWARD, U
```

## 1. Description & Usage

Removes active UPnP / NAT-PMP port forwarding mapping from the gateway router.

## 2. Syntax

```basic
UPNP.UNFORWARD ext_port [, proto$]
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 57: UPnP port unforward failed on gateway

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Network
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UPNP.UNFORWARD |
| Category | Hardware & Network |
| Syntax | UPNP.UNFORWARD ext_port [, proto$] |
| Description | Removes active UPnP / NAT-PMP port forwarding mapping from the gateway router. |
| Error Summary | Error 57: UPnP port unforward failed on gateway |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_upnp.c |
