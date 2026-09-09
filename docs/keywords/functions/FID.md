<!--
Title:        FID
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/filesystem/descriptors/fid.c
Generated:    no, hand-written
Status:       current
-->

# `FID` Keyword Reference

## Source Header

```c
// FILENAME: fid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (fid.h, string.c)
// Provides runtime implementation for the FID built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns file/device identification string (FID) or numeric position/length info (FIN) (Basic Four / BBx).

## 2. Syntax

```basic
FID(channel) / FIN(channel)
```

## 3. Code Example

```basic
10 Val = FID(channel) / FIN(channel)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File & Device Telemetry
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FID |
| Category | File & Device Telemetry |
| Syntax | FID(channel) / FIN(channel) |
| Description | Returns file/device identification string (FID) or numeric position/length info (FIN) (Basic Four / BBx). |
| Error Summary | Error 13: Type Mismatch, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/filesystem/descriptors/fid.c |
