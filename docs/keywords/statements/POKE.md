<!--
Title:        POKE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/poke.c
Generated:    no, hand-written
Status:       current
-->

# `POKE` Keyword Reference

## Source Header

```c
// FILENAME: poke.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the POKE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Writes a byte directly to virtual memory at segment:offset or to an I/O port.

## 2. Syntax

```basic
POKE offset, byte_val | OUT port, val
```

## 3. Code Example

```basic
10 REM POKE Demonstration
20 PRINT "POKE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POKE |
| Category | Variables & Memory |
| Syntax | POKE offset, byte_val \| OUT port, val |
| Description | Writes a byte directly to virtual memory at segment:offset or to an I/O port. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/poke.c |
