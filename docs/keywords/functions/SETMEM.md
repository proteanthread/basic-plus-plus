<!--
Title:        SETMEM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/setmem.c
Generated:    no, hand-written
Status:       current
-->

# `SETMEM` Keyword Reference

## Source Header

```c
// FILENAME: setmem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, memory.h)
// NEEDS: libengine (setmem.h)
// Provides runtime implementation for the SETMEM function in BASIC++.
```

## 1. Description & Usage

Adjusts the memory pool reservation and returns available RAM in bytes.

## 2. Syntax

```basic
SETMEM(bytes&)
```

## 3. Code Example

```basic
10 Val = SETMEM(bytes&)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SETMEM |
| Category | System Functions |
| Syntax | SETMEM(bytes&) |
| Description | Adjusts the memory pool reservation and returns available RAM in bytes. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/setmem.c |
