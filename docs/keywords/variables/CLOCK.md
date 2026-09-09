<!--
Title:        CLOCK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/descriptors/system_var_descriptors.c
Generated:    no, hand-written
Status:       current
-->

# `CLOCK` Keyword Reference

## Source Header

```c
// FILENAME: system_var_descriptors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (language_descriptor.h, system_var_descriptors.h)
// Implements authoritative descriptor definitions for built-in system variabl
```

## 1. Description & Usage

Local unpunctuated numeric timestamp (YYYYMMDDHHMMSS format).

## 2. Syntax

```basic
CLOCK or CLOCK()
```

## 3. Code Example

```basic
10 Val = CLOCK or CLOCK()
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Clocks & Timers
- **Subsystem**: SUBSYSTEM_CORE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CLOCK |
| Category | Clocks & Timers |
| Syntax | CLOCK or CLOCK() |
| Description | Local unpunctuated numeric timestamp (YYYYMMDDHHMMSS format). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_CORE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/runtime/descriptors/system_var_descriptors.c |
