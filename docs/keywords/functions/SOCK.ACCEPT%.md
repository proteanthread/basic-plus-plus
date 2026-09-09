<!--
Title:        SOCK.ACCEPT%
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_sock.c
Generated:    no, hand-written
Status:       current
-->

# `SOCK.ACCEPT%` Keyword Reference

## Source Header

```c
// FILENAME: func_sock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memops.h, memops.c, sock_engine.h, sock_engine.c, string.h)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (func_sock.h, string.c)
// Implements SOCK built-in functions: SOCK.OPEN%, SOCK.ACCEPT%, SOCK.RECV$, S
//
// ---- Includes ----
```

## 1. Description & Usage

Accepts an incoming connection on a listening socket handle and returns client socket handle.

## 2. Syntax

```basic
SOCK.ACCEPT%(listen_h%)
```

## 3. Code Example

```basic
10 Val = SOCK.ACCEPT%(listen_h%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5 (Illegal function call) on invalid handle

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
| Name | SOCK.ACCEPT% |
| Category | Hardware & Network |
| Syntax | SOCK.ACCEPT%(listen_h%) |
| Description | Accepts an incoming connection on a listening socket handle and returns client socket handle. |
| Error Summary | Error 5 (Illegal function call) on invalid handle |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_sock.c |
