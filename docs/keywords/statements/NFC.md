<!--
Title:        NFC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/stmt_nfc.c
Generated:    no, hand-written
Status:       current
-->

# `NFC` Keyword Reference

## Source Header

```c
// FILENAME: stmt_nfc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine
// Implements the NFC statement for RFID / NFC tag and card operations.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls external PN532 / MFRC522 NFC & 13.56 MHz RFID readers over I2C/SPI.

## 2. Syntax

```basic
NFC.INIT [addr] | NFC.SCAN uid_var$ | NFC.READ block, data_var$ | NFC.WRITE block, data$ | NFC.EMULATE uid$
```

## 3. Code Example

```basic
10 REM NFC Demonstration
20 PRINT "NFC executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & IoT
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | NFC |
| Category | Hardware & IoT |
| Syntax | NFC.INIT [addr] \| NFC.SCAN uid_var$ \| NFC.READ block, data_var$ \| NFC.WRITE block, data$ \| NFC.EMULATE uid$ |
| Description | Controls external PN532 / MFRC522 NFC & 13.56 MHz RFID readers over I2C/SPI. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/stmt_nfc.c |
