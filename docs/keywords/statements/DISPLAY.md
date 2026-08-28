# `DISPLAY` Virtual Console Output Statement

## 1. BASIC Usage and Keyword Definition

Outputs formatted text directly to the virtual terminal console device.

### Syntax Signatures:
```basic
DISPLAY expression [, expression...]
```

### Operational Notes:
- Bypasses standard line buffering for high-throughput console display.

---

## 2. Code Examples

```basic
10 DISPLAY "Virtual Console Output: ", 42, 3.14159
```
