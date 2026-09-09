<!--
Title:        PACKET.TYPE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_packet.c
Generated:    no, hand-written
Status:       current
-->

# `PACKET.TYPE$` Keyword Reference

## Source Header

```c
// FILENAME: func_packet.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memops.h, memops.c, packet_sniff.h, packet_sniff.c, string.
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (func_packet.h, string.c)
// Implements PACKET.* built-in metadata inspection functions.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns protocol type string ('TCP', 'UDP', 'RAW') of last captured packet.

## 2. Syntax

```basic
PACKET.TYPE$()
```

## 3. Code Example

```basic
10 Val = PACKET.TYPE$()
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
| Name | PACKET.TYPE$ |
| Category | Hardware & Network |
| Syntax | PACKET.TYPE$() |
| Description | Returns protocol type string ('TCP', 'UDP', 'RAW') of last captured packet. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_packet.c |
