# `PCOPY` Video Display Page Copy Statement

## 1. BASIC Usage and Keyword Definition

Copies the complete video buffer contents of one display page to another in multi-page screen modes.

### Syntax Signatures:
```basic
PCOPY source_page%, destination_page%
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Page index outside mode range.

### Operational Notes:
- Enables double-buffered, flicker-free animation.

---

## 2. Code Examples

```basic
10 SCREEN 7, , 0, 1 : REM Screen 7: Draw to page 1, display page 0
20 CIRCLE (160, 100), 50, 14
30 PCOPY 1, 0 : REM Flip page 1 onto display page 0
```
