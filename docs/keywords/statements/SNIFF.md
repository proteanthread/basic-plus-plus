<!--
Title:        SNIFF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_sniff.c
Generated:    no, hand-written
Status:       current
-->

# `SNIFF` Keyword Reference

## Source Header

```c
// FILENAME: stmt_sniff.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (packet_sniff.h, packet_sniff.c, strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, stmt_sniff.h)
// Implements SNIFF statement handler for promiscuous packet capture.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls promiscuous network packet capture and filtering.

## 2. Syntax

```basic
SNIFF [ch] [, filter$] | SNIFF ON|OFF|STOP
```

## 3. Code Example

```basic
10 REM SNIFF Demonstration
20 PRINT "SNIFF executed successfully."
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
| Name | SNIFF |
| Category | Hardware & Network |
| Syntax | SNIFF [ch] [, filter$] \| SNIFF ON\|OFF\|STOP |
| Description | Controls promiscuous network packet capture and filtering. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_sniff.c |
