<!--
Title:        INTERRUPT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/interrupt.c
Generated:    no, hand-written
Status:       current
-->

# `INTERRUPT` Keyword Reference

## Source Header

```c
// FILENAME: interrupt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, exec_internal.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (bios.h, bios.c, eval.h, eval.c, interrupt.h, map.h, map.c
// NEEDS: libengine (string.c, time.h, time.c)
// Provides runtime implementation for the INTERRUPT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage



## 2. Syntax

```basic
CALL INTERRUPT(int_num%, inregs, outregs) / INTERRUPT int_num%, inregs, outregs
```

## 3. Code Example

```basic
10 Val = CALL INTERRUPT(int_num%, inregs, outregs) / INTERRUPT int_num%, inregs, outregs
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INTERRUPT |
| Category | System & Hardware |
| Syntax | CALL INTERRUPT(int_num%, inregs, outregs) / INTERRUPT int_num%, inregs, outregs |
| Description | none |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/interrupt.c |
