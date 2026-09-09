<!--
Title:        ON_TIMER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/on_timer.c
Generated:    no, hand-written
Status:       current
-->

# `ON TIMER` Keyword Reference

## Source Header

```c
// FILENAME: on_timer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the ON_TIMER statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Establishes a periodic timer interrupt subroutine trigger.

## 2. Syntax

```basic
ON TIMER(seconds) GOSUB line_label | TIMER {ON|OFF|STOP}
```

## 3. Code Example

```basic
10 Val = ON TIMER(seconds) GOSUB line_label | TIMER {ON|OFF|STOP}
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON TIMER |
| Category | Event Trapping |
| Syntax | ON TIMER(seconds) GOSUB line_label \| TIMER {ON\|OFF\|STOP} |
| Description | Establishes a periodic timer interrupt subroutine trigger. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/on_timer.c |
