# `VMEM` Virtual Segmented Memory Statement & Function

## 1. BASIC Usage and Keyword Definition

The `VMEM` keyword provides high-level control and querying of the segmented virtual memory and bank-switching subsystem.

### Syntax Signatures:
```basic
VMEM bank_number%, segment_address&
bytes& = VMEM(selector%)
```

### Operational Rules:
- Directs virtual address mapping windows to specific 64KB banks.
- Queries virtual memory allocation and page fault metrics.

---

## 2. Code Examples

```basic
10 VMEM 1, &H1000 : REM Map Bank 1 to segment &H1000
20 PRINT "Virtual memory active bank: "; VMEM(0)
```
