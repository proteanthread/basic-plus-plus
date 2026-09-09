<!--
Title:        PACK$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/pack.c
Generated:    no, hand-written
Status:       current
-->

# `PACK$` Keyword Reference

## Source Header

```c
// FILENAME: pack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mux.h, mux.c, pack.h, string.c)
// Provides runtime implementation for the PACK built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Packs binary data values into a binary string according to format template fmt$.

## 2. Syntax

```basic
PACK$(fmt$, val1 [, val2, ...])
```

## 3. Code Example

```basic
10 Val = PACK$(fmt$, val1 [, val2, ...])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string format)

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
| Name | PACK$ |
| Category | String Functions |
| Syntax | PACK$(fmt$, val1 [, val2, ...]) |
| Description | Packs binary data values into a binary string according to format template fmt$. |
| Error Summary | Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string format) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/pack.c |
