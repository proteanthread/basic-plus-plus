<!--
Title:        MEM.FREE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/func_mem.c
Generated:    no, hand-written
Status:       current
-->

# `MEM.FREE` Keyword Reference

## Source Header

```c
// FILENAME: func_mem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements MEM.FREE and MEM.ALLOC built-in functions to query available hea
//
// ---- Includes ----
```

## 1. Description & Usage

Returns runtime_free heap memory available in bytes.

## 2. Syntax

```basic
MEM.FREE() | MEM.ALLOC()
```

## 3. Code Example

```basic
10 Val = MEM.FREE() | MEM.ALLOC()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Power
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MEM.FREE |
| Category | System & Power |
| Syntax | MEM.FREE() \| MEM.ALLOC() |
| Description | Returns runtime_free heap memory available in bytes. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/func_mem.c |
