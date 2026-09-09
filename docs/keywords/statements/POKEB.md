<!--
Title:        POKEB
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/memory/alloc.c
Generated:    no, hand-written
Status:       current
-->

# `POKEB` Keyword Reference

## Source Header

```c
// FILENAME: alloc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel
// NEEDS: libcore (alloc.h, memops.h, memops.c)
// Freestanding allocator bridge implementation.
//
// ---- Includes ----
```

## 1. Description & Usage

Writes an unsigned 8-bit byte directly to physical or virtual memory address.

## 2. Syntax

```basic
POKEB address, byte_val
```

## 3. Code Example

```basic
10 REM POKEB Demonstration
20 PRINT "POKEB executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Memory
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_UNSAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POKEB |
| Category | Hardware & Memory |
| Syntax | POKEB address, byte_val |
| Description | Writes an unsigned 8-bit byte directly to physical or virtual memory address. |
| Error Summary | Error 5: Illegal Function Call, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_UNSAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/memory/alloc.c |
