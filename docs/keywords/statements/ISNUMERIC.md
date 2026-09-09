<!--
Title:        ISNUMERIC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/types/isnumeric.c
Generated:    no, hand-written
Status:       current
-->

# `ISNUMERIC` Keyword Reference

## Source Header

```c
// FILENAME: isnumeric.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libengine (isnumeric.h, string.h, memory.h)
// Provides runtime evaluation for the ISNUMERIC function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns -1 (True) if expression evaluates to or parses as a valid numeric value, else 0 (False).

## 2. Syntax

```basic
result = ISNUMERIC(expression)
```

## 3. Code Example

```basic
10 Val = result = ISNUMERIC(e10pression)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

none

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: none
- **Safety Level**: none

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ISNUMERIC |
| Category | Introspection |
| Syntax | result = ISNUMERIC(expression) |
| Description | Returns -1 (True) if expression evaluates to or parses as a valid numeric value, else 0 (False). |
| Error Summary | none |
| Subsystem | none |
| Safety Level | none |
| Feature Type | none |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/types/isnumeric.c |
