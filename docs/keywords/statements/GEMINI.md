<!--
Title:        GEMINI
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_gemini.c
Generated:    no, hand-written
Status:       current
-->

# `GEMINI` Keyword Reference

## Source Header

```c
// FILENAME: stmt_gemini.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gemini.h, gemini.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_gemini.h, string.c
// NEEDS: libengine (vm.h)
// Implements GEMINI statement for hosting Gemini protocol capsules.
//
// ---- Includes ----
```

## 1. Description & Usage

Hosts lightweight Gemini TLS capsules or fetches text/gemini documents.

## 2. Syntax

```basic
GEMINI.SERVE [port] [, root_dir$] | GEMINI.GET$(url$)
```

## 3. Code Example

```basic
10 Val = GEMINI.SERVE [port] [, root_dir$] | GEMINI.GET$(url$)
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
| Name | GEMINI |
| Category | Network & Cloud |
| Syntax | GEMINI.SERVE [port] [, root_dir$] \| GEMINI.GET$(url$) |
| Description | Hosts lightweight Gemini TLS capsules or fetches text/gemini documents. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_gemini.c |
