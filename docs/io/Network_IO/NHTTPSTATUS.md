# NHTTPSTATUS Function Reference

The `NHTTPSTATUS` built-in function returns the HTTP/HTTPS response status code received from a remote web server on an active network I/O channel.

## Syntax

```basic
http_code% = NHTTPSTATUS(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value & Common Status Codes

| Code | Meaning | Category |
|------|---------|----------|
| **`200`** | `OK` — Request succeeded. | Success |
| **`301`** | `Moved Permanently` — Resource relocated. | Redirection |
| **`400`** | `Bad Request` — Malformed query. | Client Error |
| **`401`** | `Unauthorized` — Missing credentials. | Client Error |
| **`404`** | `Not Found` — Resource does not exist. | Client Error |
| **`500`** | `Internal Server Error` — Server crashed. | Server Error |

---

## Code Examples

### Example 1: Verifying REST API HTTP Response Code
```basic
10 OPEN "HTTP://api.example.com/status" AS #1
20 Status% = NHTTPSTATUS(1)
30 IF Status% = 200 THEN
40     PRINT "API Server Healthy! Payload: "; NREAD$(1, 1024)
50 ELSE
60     PRINT "HTTP Error: "; Status%
70 END IF
80 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NHTTPSTATUS` parses the initial HTTP response line (`HTTP/1.1 200 OK`) and returns the parsed integer status code.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NJSONQUERY_STR.md`** — JSON payload querying.
- **`NREAD_STR.md`** — Network payload reading.
- **`NINFO_STR.md`** — Server connection metadata.
