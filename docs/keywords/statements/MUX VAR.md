<!--
Title:        MUX VAR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/options/mux.c
Generated:    no, hand-written
Status:       current
-->

# `MUX VAR` Keyword Reference

## Source Header

```c
// FILENAME: mux.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pack.c, unpack.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, mux.h, string.c, vm.h)
// Provides runtime implementation for the MUX statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls channel multiplexing state for virtual devices and streams.

## 2. Syntax

```basic
MUX channel, state
```

## 3. Code Example

```basic
10 REM MUX VAR Demonstration
20 PRINT "MUX VAR executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Multiplexing & Channels
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MUX VAR |
| Category | Multiplexing & Channels |
| Syntax | MUX channel, state |
| Description | Controls channel multiplexing state for virtual devices and streams. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/options/mux.c |
