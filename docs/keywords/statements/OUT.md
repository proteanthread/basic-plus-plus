<!--
Title:        OUT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/out.c
Generated:    no, hand-written
Status:       current
-->

# `OUT` Keyword Reference

## Source Header

```c
// FILENAME: out.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (bios.h, bios.c, eval.h, eval.c, out.h, string.c)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c)
// Provides runtime implementation for the OUT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sends a byte (0-255) to a hardware or virtual I/O port address.

## 2. Syntax

```basic
OUT port, data
```

## 3. Code Example

```basic
10 REM OUT Demonstration
20 PRINT "OUT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | OUT |
| Category | System & Hardware I/O |
| Syntax | OUT port, data |
| Description | Sends a byte (0-255) to a hardware or virtual I/O port address. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/out.c |
