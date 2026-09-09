<!--
Title:        ON_PLAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/events/events.c
Generated:    no, hand-written
Status:       current
-->

# `ON_PLAY` Keyword Reference

## Source Header

```c
// FILENAME: events.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for ON_PLAY.
```

## 1. Description & Usage

Configures continuous background sound queue threshold trapping for music playback.

## 2. Syntax

```basic
ON PLAY(n) GOSUB line | PLAY {ON|OFF|STOP}
```

## 3. Code Example

```basic
10 Val = ON PLAY(n) GOSUB line | PLAY {ON|OFF|STOP}
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON_PLAY |
| Category | Event Trapping |
| Syntax | ON PLAY(n) GOSUB line \| PLAY {ON\|OFF\|STOP} |
| Description | Configures continuous background sound queue threshold trapping for music playback. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/events/events.c |
