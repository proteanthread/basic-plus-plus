<!--
Title:        MEMMAP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/memory/alloc.c
Generated:    no, hand-written
Status:       current
-->

# `MEMMAP` Keyword Reference

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

Queries memory map descriptors for the specified physical or virtual memory region index.

## 2. Syntax

```basic
MEMMAP(region_idx%)
```

## 3. Code Example

```basic
10 Val = MEMMAP(region_id10%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Memory
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MEMMAP |
| Category | Hardware & Memory |
| Syntax | MEMMAP(region_idx%) |
| Description | Queries memory map descriptors for the specified physical or virtual memory region index. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/memory/alloc.c |
