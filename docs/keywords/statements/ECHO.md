<!--
Title:        ECHO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/echo.c
Generated:    no, hand-written
Status:       current
-->

# `ECHO` Keyword Reference

## Source Header

```c
// FILENAME: echo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (session.h, session.c, string.h)
// NEEDS: libengine (echo.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the ECHO statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables terminal input character echo (DEC / Timesharing).

## 2. Syntax

```basic
ECHO [ON | OFF]
```

## 3. Code Example

```basic
10 REM ECHO Demonstration
20 PRINT "ECHO executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console & Keyboard
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ECHO |
| Category | Console & Keyboard |
| Syntax | ECHO [ON \| OFF] |
| Description | Enables terminal input character echo (DEC / Timesharing). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/echo.c |
