<!--
Title:        UNPACK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/unpack.c
Generated:    no, hand-written
Status:       current
-->

# `UNPACK` Keyword Reference

## Source Header

```c
// FILENAME: unpack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mux.h, mux.c, string.c, unpack.h)
// Provides runtime implementation for the UNPACK built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Unpacks binary data from bin_str$ according to format template fmt$.

## 2. Syntax

```basic
UNPACK(fmt$, bin_str$)
```

## 3. Code Example

```basic
10 Val = UNPACK(fmt$, bin_str$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string arguments)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UNPACK |
| Category | String Functions |
| Syntax | UNPACK(fmt$, bin_str$) |
| Description | Unpacks binary data from bin_str$ according to format template fmt$. |
| Error Summary | Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string arguments) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/unpack.c |
