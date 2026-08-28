# `LPRINT` Line Printer Output Statement

## 1. BASIC Usage and Keyword Definition

Prints text and formatted data directly to the default line printer device (LPT1).

### Syntax Signatures:
```basic
LPRINT [USING format$;] [expression [,;] ...]
```

### Error Handling & Boundary Conditions:
- **Error 25 (ERR_DEVICE_FAULT)**: Printer not ready.

### Operational Notes:
- Formatted identically to standard PRINT and PRINT USING statements.
- Routes output through the built-in virtual text-to-PDF pseudo-printer driver.
- Outputs standard PDF 1.4 documents formatted with built-in Courier font (10pt, 12pt line leading, Letter/A4 pagination).
- Automatically saves to `<program_name>.pdf` in the current working directory, or `OUTPUT.PDF` if no program file was loaded.
- Form Feed (`CHR$(12)` or `\f`) automatically splits content across pages.

---

## 2. Code Examples

```basic
10 LPRINT "--- INVOICE REPORT ---"
20 LPRINT USING "Item Total: $###.##"; 125.50
30 LPRINT CHR$(12) : REM Form feed to next page
40 LPRINT "Page 2 Content"
```
