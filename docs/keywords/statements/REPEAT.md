<!--
Title:        REPEAT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/repeat.c
Generated:    no, hand-written
Status:       current
-->

# `REPEAT` Keyword Reference

## Source Header

```c
// FILENAME: repeat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (repeat.h, string.c)
// Provides runtime implementation for the REPEAT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Initiates a BASIC09 post-test REPEAT...UNTIL loop block.

## 2. Syntax

```basic
REPEAT
```

## 3. Code Example

```basic
10 REM REPEAT Demonstration
20 PRINT "REPEAT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 14: Loop Stack Overflow

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Looping / Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REPEAT |
| Category | Looping / Control Flow |
| Syntax | REPEAT |
| Description | Initiates a BASIC09 post-test REPEAT...UNTIL loop block. |
| Error Summary | Error 2: Syntax Error, Error 14: Loop Stack Overflow |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/repeat.c |
