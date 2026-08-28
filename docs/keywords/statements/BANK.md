# `BANK` Memory Bank Selection Statement

## 1. BASIC Usage and Keyword Definition

The `BANK` statement selects the active 64KB memory bank in BASIC++'s virtual segmented memory architecture (`vmem`). Subsequent `PEEK`, `POKE`, `PEEKB`, `POKEB`, and `DEF SEG` operations operate within the selected bank unless overridden.

### Syntax Signatures:
```basic
BANK bank_number%
current_bank% = BANK()
```

### Operational Rules:
- **Bank Range**: $0 \le \text{bank\_number} < \text{RAMBANKS}()$ (defaults to 16 virtual 64KB banks = 1MB virtual address space, scalable up to 1024 banks).
- **Security Check**: Requires memory access capability (`CAP_MEM`). Unauthorized access triggers Error 70 (`ERR_PERMISSION_DENIED`).
- **Bank 0**: Default primary system bank containing virtual BIOS Data Area (BDA) and system buffers.

---

## 2. Code Examples

```basic
10 PRINT "Available memory banks: "; RAMBANKS()
20 BANK 1 : REM Switch to Bank 1
30 POKE &H1000, &H42
40 PRINT "Read back from Bank 1: &H"; HEX$(PEEK(&H1000))
50 BANK 0 : REM Return to Bank 0
```
