<!--
Title:        PEEK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/peek.c
Generated:    no, hand-written
Status:       current
-->

# `PEEK` Keyword Reference

## Source Header

```c
// FILENAME: peek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the PEEK built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads a byte (0-255) from virtual memory address addr.

## 2. Syntax

```basic
PEEK(addr)
```

## 3. Code Example

```basic
10 Val = PEEK(addr)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address)

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
| Name | PEEK |
| Category | System Functions |
| Syntax | PEEK(addr) |
| Description | Reads a byte (0-255) from virtual memory address addr. |
| Error Summary | Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/peek.c |
