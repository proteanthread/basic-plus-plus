# `LLIST` Printer Program Source Listing Command

## 1. BASIC Usage and Keyword Definition

Outputs program source lines from memory directly to the default line printer device (LPT1).

### Syntax Signatures:
```basic
LLIST [start_line%] - [end_line%]
```

### Error Handling & Boundary Conditions:
- **Error 25 (ERR_DEVICE_FAULT)**: Printer is offline or out of paper.

### Operational Notes:
- Routes through the built-in virtual text-to-PDF pseudo-printer driver.
- Outputs standard PDF 1.4 documents formatted with built-in Courier font (10pt, 12pt line leading, Letter/A4 pagination).
- Automatically saves to `<program_name>.pdf` in the current working directory, or `OUTPUT.PDF` if no program file was loaded.

---

## 2. Code Examples

```basic
LLIST         : REM Prints entire program listing to PDF
LLIST 10-50   : REM Prints lines 10 to 50 to PDF
LLIST 100-    : REM Prints from line 100 to end of program to PDF
```
