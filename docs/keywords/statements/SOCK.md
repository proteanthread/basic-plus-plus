<!--
Title:        SOCK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_sock.c
Generated:    no, hand-written
Status:       current
-->

# `SOCK` Keyword Reference

## Source Header

```c
// FILENAME: stmt_sock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (sock_engine.h, sock_engine.c, strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, stmt_sock.h)
// Implements BSD SOCK statement handlers.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls BSD-style network sockets for TCP/UDP listening, binding, transmission, and teardown.

## 2. Syntax

```basic
SOCK.BIND h, port | SOCK.LISTEN h | SOCK.SEND h, data$ | SOCK.CLOSE h | SOCK.SETSOCKOPT h, opt$, v
```

## 3. Code Example

```basic
10 REM SOCK Demonstration
20 PRINT "SOCK executed successfully."
```

## 4. Error Conditions

Error 5: Unknown SOCK sub-command

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
| Name | SOCK |
| Category | Hardware & Network |
| Syntax | SOCK.BIND h, port \| SOCK.LISTEN h \| SOCK.SEND h, data$ \| SOCK.CLOSE h \| SOCK.SETSOCKOPT h, opt$, v |
| Description | Controls BSD-style network sockets for TCP/UDP listening, binding, transmission, and teardown. |
| Error Summary | Error 5: Unknown SOCK sub-command |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_sock.c |
