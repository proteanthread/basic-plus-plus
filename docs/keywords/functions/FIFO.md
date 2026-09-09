<!--
Title:        FIFO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/functions/system/func_udx.c
Generated:    no, hand-written
Status:       current
-->

# `FIFO` Keyword Reference

## Source Header

```c
// FILENAME: func_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: libreg (reg_buffer.h, reg_udx.h, reg_hw.h), runtime (funcreg.h, lang
// Implements freestanding UDX, FIFO, LIFO functions in BASIC++.
```

## 1. Description & Usage

First-class First In, First Out buffer operation function.

## 2. Syntax

```basic
FIFO(op$, [val]) | FIFO.PUSH(val) | FIFO.POP()
```

## 3. Code Example

```basic
10 Val = FIFO(op$, [val]) | FIFO.PUSH(val) | FIFO.POP()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FIFO |
| Category | System & Hardware |
| Syntax | FIFO(op$, [val]) \| FIFO.PUSH(val) \| FIFO.POP() |
| Description | First-class First In, First Out buffer operation function. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/functions/system/func_udx.c |
