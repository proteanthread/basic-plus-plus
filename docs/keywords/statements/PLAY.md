<!--
Title:        PLAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/sound/synthesis/play.c
Generated:    no, hand-written
Status:       current
-->

# `PLAY` Keyword Reference

## Source Header

```c
// FILENAME: play.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, play.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PLAY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Plays musical notes using MML (Music Macro Language) command string.

## 2. Syntax

```basic
PLAY command_string
```

## 3. Code Example

```basic
10 REM PLAY Demonstration
20 PRINT "PLAY executed successfully."
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
| Name | PLAY |
| Category | Sound & Audio |
| Syntax | PLAY command_string |
| Description | Plays musical notes using MML (Music Macro Language) command string. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/sound/synthesis/play.c |
