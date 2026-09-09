<!--
Title:        GEMINI.BROWSE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_gemini_browse.c
Generated:    no, hand-written
Status:       current
-->

# `GEMINI.BROWSE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_gemini_browse.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gemini.h, gemini.c, string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, stmt_gemini_browse.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Implements GEMINI.BROWSE interactive TUI capsule browser.
//
// ---- Includes ----
```

## 1. Description & Usage

Fetches and displays a Gemini protocol capsule page in the terminal.

## 2. Syntax

```basic
GEMINI.BROWSE [url$]
```

## 3. Code Example

```basic
10 REM GEMINI.BROWSE Demonstration
20 PRINT "GEMINI.BROWSE executed successfully."
```

## 4. Error Conditions

None

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
| Name | GEMINI.BROWSE |
| Category | Hardware & Network |
| Syntax | GEMINI.BROWSE [url$] |
| Description | Fetches and displays a Gemini protocol capsule page in the terminal. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_gemini_browse.c |
