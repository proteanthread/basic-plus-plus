<!--
Title:        UDX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_udx.c
Generated:    no, hand-written
Status:       current
-->

# `UDX` Keyword Reference

## Source Header

```c
// FILENAME: stmt_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: libreg (reg_buffer.h, reg_udx.h, reg_hw.h), eval/eval.h, vm/vm.h
// Implements runtime statements for Universal Data Exchange (UDX, XCHG, PUSH,
```

## 1. Description & Usage

Manages inter-process and inter-function data exchange routing and channels across the UDX bus.

## 2. Syntax

```basic
UDX TRANSFER src, dst | UDX CHANNEL name$, discipline$
```

## 3. Code Example

```basic
10 REM UDX Demonstration
20 PRINT "UDX executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UDX |
| Category | System & Hardware |
| Syntax | UDX TRANSFER src, dst \| UDX CHANNEL name$, discipline$ |
| Description | Manages inter-process and inter-function data exchange routing and channels across the UDX bus. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_udx.c |
