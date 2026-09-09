<!--
Title:        VOICE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/sound/synthesis/voice.c
Generated:    no, hand-written
Status:       current
-->

# `VOICE` Keyword Reference

## Source Header

```c
// FILENAME: voice.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, voice.h
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the VOICE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures synthesizer voice envelope parameters (ADSR) for multi-channel sound output.

## 2. Syntax

```basic
VOICE channel, waveform, attack, decay, sustain, release
```

## 3. Code Example

```basic
10 REM VOICE Demonstration
20 PRINT "VOICE executed successfully."
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
| Name | VOICE |
| Category | Sound & Audio |
| Syntax | VOICE channel, waveform, attack, decay, sustain, release |
| Description | Configures synthesizer voice envelope parameters (ADSR) for multi-channel sound output. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/sound/synthesis/voice.c |
