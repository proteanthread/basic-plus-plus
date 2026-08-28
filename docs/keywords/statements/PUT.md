# `PUT` Random File Record Write or Graphics Blit Statement

## 1. BASIC Usage and Keyword Definition

Writes a record buffer to a random-access file, or blits a sprite image from an array onto the screen.

### Syntax Signatures:
```basic
PUT [#]filenum% [, record_number&]
PUT (x1%, y1%), array% [, {PSET | PRESET | AND | OR | XOR}]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel not open.

### Operational Notes:
- XOR blitting enables non-destructive sprite animation.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "DATA.DAT", 64
20 PUT #1, 1 : REM Write record 1
30 CLOSE #1
```
