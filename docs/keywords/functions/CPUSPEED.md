<!--
Title:        CPUSPEED
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_cpuspeed.c
Generated:    no, hand-written
Status:       current
-->

# `CPUSPEED` Keyword Reference

## Source Header

```c
// FILENAME: stmt_cpuspeed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Statement implementation for CPUSPEED statement (e.g. CPUSPEED EMULATE).
```

## 1. Description & Usage

Configures authentic CPU and I/O speed emulation for vintage folklore systems.

## 2. Syntax

```basic
CPUSPEED EMULATE <model> [, IO] | CPUSPEED EMULATE OFF | CPUSPEED{...}
```

## 3. Code Example

```basic
10 REM CPUSPEED Demonstration
20 PRINT "CPUSPEED executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Peripherals
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CPUSPEED |
| Category | Hardware & Peripherals |
| Syntax | CPUSPEED EMULATE <model> [, IO] \| CPUSPEED EMULATE OFF \| CPUSPEED{...} |
| Description | Configures authentic CPU and I/O speed emulation for vintage folklore systems. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_cpuspeed.c |
