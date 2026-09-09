<!--
Title:        REBOOT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_reboot.c
Generated:    no, hand-written
Status:       current
-->

# `REBOOT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_reboot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements REBOOT and RESET statements for microcontroller restart.
//
// ---- Includes ----
```

## 1. Description & Usage

Triggers a hardware system restart on microcontroller target.

## 2. Syntax

```basic
REBOOT | RESET
```

## 3. Code Example

```basic
10 REM REBOOT Demonstration
20 PRINT "REBOOT executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Power
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REBOOT |
| Category | System & Power |
| Syntax | REBOOT \| RESET |
| Description | Triggers a hardware system restart on microcontroller target. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_reboot.c |
