<!--
Title:        GOPHER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_gopher.c
Generated:    no, hand-written
Status:       current
-->

# `GOPHER` Keyword Reference

## Source Header

```c
// FILENAME: stmt_gopher.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gopher.h, gopher.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_gopher.h, string.c
// NEEDS: libengine (vm.h)
// Implements GOPHER statement for hosting RFC 1436 Gopher menus and servers.
//
// ---- Includes ----
```

## 1. Description & Usage

Hosts RFC 1436 Gopher directory menus or fetches Gopher items.

## 2. Syntax

```basic
GOPHER.SERVE [port] [, root_dir$] | GOPHER.GET$(url$)
```

## 3. Code Example

```basic
10 Val = GOPHER.SERVE [port] [, root_dir$] | GOPHER.GET$(url$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Network & Cloud
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | GOPHER |
| Category | Network & Cloud |
| Syntax | GOPHER.SERVE [port] [, root_dir$] \| GOPHER.GET$(url$) |
| Description | Hosts RFC 1436 Gopher directory menus or fetches Gopher items. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_gopher.c |
