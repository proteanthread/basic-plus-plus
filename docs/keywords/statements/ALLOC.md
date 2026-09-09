<!--
Title:        ALLOC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/memory/alloc.c
Generated:    no, hand-written
Status:       current
-->

# `ALLOC` Keyword Reference

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

Allocates a contiguous raw byte buffer from the system memory arena and returns handle/pointer.

## 2. Syntax

```basic
ALLOC(size_bytes) | ALLOC var, size
```

## 3. Code Example

```basic
10 Val = ALLOC(size_bytes) | ALLOC var, size
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 7: Out of Memory

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Systems Standard
- **Since Version**: 6.5.0
- **Category**: Hardware & Memory
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ALLOC |
| Category | Hardware & Memory |
| Syntax | ALLOC(size_bytes) \| ALLOC var, size |
| Description | Allocates a contiguous raw byte buffer from the system memory arena and returns handle/pointer. |
| Error Summary | Error 7: Out of Memory |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Systems Standard |
| Since Version | 6.5.0 |
| Source File | engine/src/runtime/memory/alloc.c |
