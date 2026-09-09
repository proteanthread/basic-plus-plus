<!--
Title:        NET.CONFIG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_net_config.c
Generated:    no, hand-written
Status:       current
-->

# `NET.CONFIG` Keyword Reference

## Source Header

```c
// FILENAME: stmt_net_config.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, events_net.h, events_net.c)
// NEEDS: libengine (stmt_net_config.h, stmt_nil_bead.h, stmt_nil_bead.c)
// Implements NET.CONFIG statement for network interface and IP setup.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures network interface TCP/IP settings or controls network event trapping.

## 2. Syntax

```basic
NET.CONFIG iface$, ip$, netmask$, gateway$, dns$ | NET ON|OFF|STOP
```

## 3. Code Example

```basic
10 REM NET.CONFIG Demonstration
20 PRINT "NET.CONFIG executed successfully."
```

## 4. Error Conditions

Error 5: Syntax error in NET statement

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
| Name | NET.CONFIG |
| Category | Hardware & Network |
| Syntax | NET.CONFIG iface$, ip$, netmask$, gateway$, dns$ \| NET ON\|OFF\|STOP |
| Description | Configures network interface TCP/IP settings or controls network event trapping. |
| Error Summary | Error 5: Syntax error in NET statement |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_net_config.c |
