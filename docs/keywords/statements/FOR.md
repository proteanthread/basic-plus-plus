<!--
Title:        FOR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/counter/for.c
Generated:    no, hand-written
Status:       current
-->

# `FOR` Keyword Reference

## Source Header

```c
// FILENAME: for.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the FOR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Initiates a counter-controlled loop block executing statements until var reaches end.

## 2. Syntax

```basic
FOR var = start TO end [STEP step]
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 26: FOR Without NEXT

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
| Name | FOR |
| Category | Looping / Control Flow |
| Syntax | FOR var = start TO end [STEP step] |
| Description | Initiates a counter-controlled loop block executing statements until var reaches end. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 26: FOR Without NEXT |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/counter/for.c |
