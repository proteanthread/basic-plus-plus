<!--
Title:        TIM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/time/func_tim.c
Generated:    no, hand-written
Status:       current
-->

# `TIM` Keyword Reference

## Source Header

```c
// FILENAME: func_tim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, func_tim.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the HP 2000/3000 TIM built-in function 
```

## 1. Description & Usage

Returns HP 2000/3000 time components: 0=sec, 1=min, 2=hour, 3=day, 4=month, 5=year, 6=day-of-week (1-7), 7=day-of-year (1-366), 8=millisecond (0-999).

## 2. Syntax

```basic
TIM(n)
```

## 3. Code Example

```basic
10 Val = TIM(n)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | TIM |
| Category | System Functions |
| Syntax | TIM(n) |
| Description | Returns HP 2000/3000 time components: 0=sec, 1=min, 2=hour, 3=day, 4=month, 5=year, 6=day-of-week (1-7), 7=day-of-year (1-366), 8=millisecond (0-999). |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/time/func_tim.c |
