# `IOCTL` Device Control Statement & Function

## 1. BASIC Usage and Keyword Definition

The `IOCTL` statement and `IOCTL$` function transmit and query device-specific control strings and hardware configuration commands to active virtual device drivers or file channels.

### Syntax Signatures:
```basic
IOCTL [#]channel_number%, control_string$
status$ = IOCTL$([#]channel_number%)
```

### Operational Rules:
- Sends raw device configuration commands (baud rate, parity, screen modes, buffer flush) to drivers.

---

## 2. Code Examples

```basic
10 OPEN "COM1:9600,N,8,1" AS #1
20 IOCTL #1, "BAUD=115200"
30 PRINT "Device status: "; IOCTL$(#1)
40 CLOSE #1
```
