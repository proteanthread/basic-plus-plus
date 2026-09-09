<!--
Title:        CRYPTO.DECRYPT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/security/func_crypto.c
Generated:    no, hand-written
Status:       current
-->

# `CRYPTO.DECRYPT$` Keyword Reference

## Source Header

```c
// FILENAME: func_crypto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (crypto_engine.h, crypto_engine.c, memops.h, memops.c)
// NEEDS: libcore (string.h, strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (string.c)
// NEEDS: libkernel (func_crypto.h)
// Implements CRYPTO.* built-in functions: CRYPTO.ENCRYPT$, CRYPTO.DECRYPT$, C
//
// ---- Includes ----
```

## 1. Description & Usage

Decrypts hex-encoded ciphertext using symmetric key and returns plaintext.

## 2. Syntax

```basic
CRYPTO.DECRYPT$(key$, ciphertext_hex$)
```

## 3. Code Example

```basic
10 Val = CRYPTO.DECRYPT$(key$, cipherte10t_he10$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Security & Encryption
- **Subsystem**: SUBSYSTEM_SERVER
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CRYPTO.DECRYPT$ |
| Category | Security & Encryption |
| Syntax | CRYPTO.DECRYPT$(key$, ciphertext_hex$) |
| Description | Decrypts hex-encoded ciphertext using symmetric key and returns plaintext. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_SERVER |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/security/func_crypto.c |
