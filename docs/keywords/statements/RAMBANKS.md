# `RAMBANKS` Available Memory Banks Function

## 1. BASIC Usage and Function Definition

The `RAMBANKS` function returns the total number of 64KB memory banks available in the virtual segmented memory subsystem.

### Syntax Signatures:
```basic
count% = RAMBANKS()
count% = RAMBANKS
```

### Operational Rules:
- Returns the maximum bank capacity configured for the active VM target (e.g. 16 banks for 1MB virtual PC space, up to 1024 banks for extended virtual machines).

---

## 2. Code Examples

```basic
10 TOTAL_BANKS = RAMBANKS()
20 PRINT "Total 64KB virtual RAM banks available: "; TOTAL_BANKS
30 PRINT "Total addressable virtual memory: "; TOTAL_BANKS * 64; " KB"
```
