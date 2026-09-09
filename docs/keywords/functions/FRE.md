<!--
Title:        FRE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/fre.c
Generated:    no, hand-written
Status:       current
-->

# `FRE` Keyword Reference

## Source Header

```c
// FILENAME: fre.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (fre.h, string.c)
// Provides runtime implementation for the FRE built-in function in BASIC++.
```

## 1. Description & Usage

Returns available memory bytes for subsystem pools (-1: vars, -2: strings, -3: stack, -4: files, -5: disk, -6: vdev, -7: vram, -8: net, -9: audio, 0: heap, 1..64: 1MB RAMBanks).

## 2. Syntax

```basic
FRE([pool_code])
```

## 3. Code Example

```basic
10 Val = FRE([pool_code])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

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
| Name | FRE |
| Category | System Functions |
| Syntax | FRE([pool_code]) |
| Description | Returns available memory bytes for subsystem pools (-1: vars, -2: strings, -3: stack, -4: files, -5: disk, -6: vdev, -7: vram, -8: net, -9: audio, 0: heap, 1..64: 1MB RAMBanks). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/fre.c |
