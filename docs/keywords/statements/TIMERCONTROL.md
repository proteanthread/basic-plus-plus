<!--
Title:        TIMERCONTROL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/timercontrol.c
Generated:    no, hand-written
Status:       current
-->

# `TIMERCONTROL` Keyword Reference

## Source Header

```c
// FILENAME: timercontrol.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, timercontrol.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TIMERCONTROL statement in BASIC++.
```

## 1. Description & Usage

Configures an interactive background timer event control.

## 2. Syntax

```basic
TIMERCONTROL interval_ms%, enabled%
```

## 3. Code Example

```basic
10 REM TIMERCONTROL Demonstration
20 PRINT "TIMERCONTROL executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: User Interface
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TIMERCONTROL |
| Category | User Interface |
| Syntax | TIMERCONTROL interval_ms%, enabled% |
| Description | Configures an interactive background timer event control. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/timercontrol.c |
