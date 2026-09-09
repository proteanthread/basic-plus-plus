<!--
Title:        INP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/inp.c
Generated:    no, hand-written
Status:       current
-->

# `INP` Keyword Reference

## Source Header

```c
// FILENAME: inp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (inp.h, string.c)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c)
// Provides runtime implementation for the INP built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads a byte (0-255) from hardware I/O port address.

## 2. Syntax

```basic
INP(port)
```

## 3. Code Example

```basic
10 Val = INP(port)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (port out of bounds), Error 13: Type Mismatch (expects numeric port)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INP |
| Category | System Functions |
| Syntax | INP(port) |
| Description | Reads a byte (0-255) from hardware I/O port address. |
| Error Summary | Error 5: Illegal Function Call (port out of bounds), Error 13: Type Mismatch (expects numeric port) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/inp.c |
