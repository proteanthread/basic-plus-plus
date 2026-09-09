<!--
Title:        ON_GOTO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/exec/exec_dispatch.c
Generated:    no, hand-written
Status:       current
-->

# `ON_GOTO` Keyword Reference

## Source Header

```c
// FILENAME: exec_dispatch.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements bytecode virtual machine execution and state for exec_dispatch.
//
// ---- Includes ----
```

## 1. Description & Usage

Branches to one of several jump target lines based on the 1-based integer evaluation of expr.

## 2. Syntax

```basic
ON expr GOTO line1 [, line2, ...]
```

## 3. Code Example

```basic
10 REM ON_GOTO Demonstration
20 PRINT "ON_GOTO executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON_GOTO |
| Category | Control Flow |
| Syntax | ON expr GOTO line1 [, line2, ...] |
| Description | Branches to one of several jump target lines based on the 1-based integer evaluation of expr. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/exec/exec_dispatch.c |
