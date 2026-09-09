<!--
Title:        WHILE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/while.c
Generated:    no, hand-written
Status:       current
-->

# `WHILE` Keyword Reference

## Source Header

```c
// FILENAME: while.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the WHILE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Executes a series of statements in a loop as long as condition evaluates to non-zero (true).

## 2. Syntax

```basic
WHILE condition
```

## 3. Code Example

```basic
10 C = 1
20 WHILE C <= 3
30   PRINT C : C = C + 1
40 WEND
```

## 4. Error Conditions

Error 2: Syntax Error, Error 30: WHILE Without WEND

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
| Name | WHILE |
| Category | Looping / Control Flow |
| Syntax | WHILE condition |
| Description | Executes a series of statements in a loop as long as condition evaluates to non-zero (true). |
| Error Summary | Error 2: Syntax Error, Error 30: WHILE Without WEND |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/while.c |
