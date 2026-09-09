<!--
Title:        SOUND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/sound/synthesis/sound.c
Generated:    no, hand-written
Status:       current
-->

# `SOUND` Keyword Reference

## Source Header

```c
// FILENAME: sound.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, sound.h, string.c, vm.h
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SOUND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Generates a tone of specified frequency in Hertz for specified duration in clock ticks.

## 2. Syntax

```basic
SOUND frequency, duration
```

## 3. Code Example

```basic
10 REM SOUND Demonstration
20 PRINT "SOUND executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Sound & Audio
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SOUND |
| Category | Sound & Audio |
| Syntax | SOUND frequency, duration |
| Description | Generates a tone of specified frequency in Hertz for specified duration in clock ticks. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/sound/synthesis/sound.c |
