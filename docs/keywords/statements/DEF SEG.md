<!--
Title:        DEF SEG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/def_seg.c
Generated:    no, hand-written
Status:       current
-->

# `DEF SEG` Keyword Reference

## Source Header

```c
// FILENAME: def_seg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (def_seg.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the DEF_SEG statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines the current 16-bit memory segment for PEEK, POKE, BLOAD, and BSAVE.

## 2. Syntax

```basic
DEF SEG [= address%]
```

## 3. Code Example

```basic
10 REM DEF SEG Demonstration
20 PRINT "DEF SEG executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEF SEG |
| Category | System & Memory |
| Syntax | DEF SEG [= address%] |
| Description | Defines the current 16-bit memory segment for PEEK, POKE, BLOAD, and BSAVE. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/def_seg.c |
