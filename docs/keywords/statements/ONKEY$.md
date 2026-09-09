<!--
Title:        ONKEY$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/events/events.c
Generated:    no, hand-written
Status:       current
-->

# `ONKEY$` Keyword Reference

## Source Header

```c
// FILENAME: events.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for ONKEY$.
```

## 1. Description & Usage

Configures event trapping for specific character keypress sequences.

## 2. Syntax

```basic
ONKEY$(key_code$) GOSUB line
```

## 3. Code Example

```basic
10 Val = ONKEY$(key_code$) GOSUB line
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

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
| Name | ONKEY$ |
| Category | Event Trapping |
| Syntax | ONKEY$(key_code$) GOSUB line |
| Description | Configures event trapping for specific character keypress sequences. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/events/events.c |
