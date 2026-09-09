<!--
Title:        RBYTE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/gpib.c
Generated:    no, hand-written
Status:       current
-->

# `RBYTE` Keyword Reference

## Source Header

```c
// FILENAME: gpib.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, sys_fn.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h)
// Provides runtime implementation for Tektronix 4050 GPIB I/O statements and 
//
// ---- Includes ----
```

## 1. Description & Usage

Receives raw bytes across GPIB IEEE-488 bus from instrumentation (Tektronix 4050 series).

## 2. Syntax

```basic
RBYTE [@dev [, sec] :] var1 [, var2 ...] | RBYTE[dev, sec] var1 ...
```

## 3. Code Example

```basic
10 REM RBYTE Demonstration
20 PRINT "RBYTE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RBYTE |
| Category | Hardware & I/O |
| Syntax | RBYTE [@dev [, sec] :] var1 [, var2 ...] \| RBYTE[dev, sec] var1 ... |
| Description | Receives raw bytes across GPIB IEEE-488 bus from instrumentation (Tektronix 4050 series). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/gpib.c |
