<!--
Title:        NET.UNPACK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_nil_bead.c
Generated:    no, hand-written
Status:       current
-->

# `NET.UNPACK` Keyword Reference

## Source Header

```c
// FILENAME: stmt_nil_bead.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_net_config.c)
// NEEDS: libcore (nil_bead.h, nil_bead.c, string.h, strings.h, strings.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, stmt_nil_bead.h, string.c)
// Implements NET.UNPACK and NIL.UNPACK statement handlers.
//
// ---- Includes ----
```

## 1. Description & Usage

Deserializes an RFC 51 compact bead byte stream into a destination variable.

## 2. Syntax

```basic
NET.UNPACK packet$, dest_var
```

## 3. Code Example

```basic
10 REM NET.UNPACK Demonstration
20 PRINT "NET.UNPACK executed successfully."
```

## 4. Error Conditions

Error 2: Expected destination variable, Error 13: Type mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Network
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NET.UNPACK |
| Category | Hardware & Network |
| Syntax | NET.UNPACK packet$, dest_var |
| Description | Deserializes an RFC 51 compact bead byte stream into a destination variable. |
| Error Summary | Error 2: Expected destination variable, Error 13: Type mismatch |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_nil_bead.c |
