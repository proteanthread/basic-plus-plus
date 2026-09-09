<!--
Title:        MUTEX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_mutex.c
Generated:    no, hand-written
Status:       current
-->

# `MUTEX` Keyword Reference

## Source Header

```c
// FILENAME: stmt_mutex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libengine (eval.h, lexer.h, stmt_mutex.h, vm.h)
// Provides runtime implementation for MUTEX concurrency statement and functio
//
// ---- Includes ----
```

## 1. Description & Usage

Creates, locks, or unlocks a virtual concurrency mutex.

## 2. Syntax

```basic
m = MUTEX(name$) | MUTEX_LOCK(m) | MUTEX_UNLOCK(m)
```

## 3. Code Example

```basic
10 Val = m = MUTEX(name$) | MUTEX_LOCK(m) | MUTEX_UNLOCK(m)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Concurrency
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MUTEX |
| Category | Concurrency |
| Syntax | m = MUTEX(name$) \| MUTEX_LOCK(m) \| MUTEX_UNLOCK(m) |
| Description | Creates, locks, or unlocks a virtual concurrency mutex. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_mutex.c |
