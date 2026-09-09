<!--
Title:        BEEP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/sound/synthesis/beep.c
Generated:    no, hand-written
Status:       current
-->

# `BEEP` Keyword Reference

## Source Header

```c
// FILENAME: beep.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (beep.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the BEEP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Emits standard 800 Hz speaker beep tones for count repetitions with optional delay in seconds (default 1 beep, 1.0s delay).

## 2. Syntax

```basic
BEEP [count [, delay]]
```

## 3. Code Example

```basic
10 REM BEEP Demonstration
20 PRINT "BEEP executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | BEEP |
| Category | Sound & Audio |
| Syntax | BEEP [count [, delay]] |
| Description | Emits standard 800 Hz speaker beep tones for count repetitions with optional delay in seconds (default 1 beep, 1.0s delay). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/sound/synthesis/beep.c |
