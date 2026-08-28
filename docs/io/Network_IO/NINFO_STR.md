# NINFO$ Function Reference

The `NINFO$` built-in string function queries connection metadata (remote IP address, port numbers, protocol type) from an active network socket I/O channel.

## Syntax

```basic
info_val$ = NINFO$(channel%, property_name$)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).
- **`property_name$`** — A string expression specifying which socket property to query:
  - **`"PEER_IP"`** — Remote host IPv4/IPv6 address string (e.g. `"192.168.1.50"`).
  - **`"PEER_PORT"`** — Remote host port number string (e.g. `"8080"`).
  - **`"LOCAL_IP"`** — Local machine bound interface IP string.
  - **`"LOCAL_PORT"`** — Local machine bound ephemeral port string.
  - **`"PROTOCOL"`** — Socket protocol type (`"TCP"`, `"UDP"`, `"HTTP"`, `"HTTPS"`).

## Return Value

- Returns a **string** (`VAL_STRING`) containing the requested socket metadata property.

---

## Code Examples

### Example 1: Logging Inbound Socket Client Details
```basic
10 OPEN "TCP:SERVER:9000" AS #1
20 PRINT "Client Connected from: "; NINFO$(1, "PEER_IP"); ":"; NINFO$(1, "PEER_PORT")
30 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NINFO$` calls `getpeername()` and `getsockname()`, formatting the IP strings with `inet_ntop()`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 5 | Illegal Function Call (`ERR_ILLEGAL_FUNCTION_CALL`) | Unrecognized property name |
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NCONNECTED.md`** — Connection flag.
- **`NHTTPSTATUS.md`** — HTTP response code.
- **`NREAD_STR.md`** — Network read.
