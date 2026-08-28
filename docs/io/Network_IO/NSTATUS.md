# NSTATUS Function Reference

The `NSTATUS` built-in function returns the protocol lifecycle state code of an active network socket I/O channel.

## Syntax

```basic
state_code% = NSTATUS(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value & State Codes

| Code | State Name | Description |
|------|------------|-------------|
| **`0`** | `CLOSED` | Socket is idle, unallocated, or closed. |
| **`1`** | `LISTENING` | Server socket accepting incoming connections. |
| **`2`** | `CONNECTING` | Handshake in progress. |
| **`3`** | `CONNECTED` | Active established TCP/HTTP session. |
| **`4`** | `CLOSING` | FIN packet received / shutdown pending. |
| **`5`** | `ERROR` | Socket encountered unrecoverable network error. |

---

## Code Examples

### Example 1: Polling Socket Lifecycle State
```basic
10 OPEN "TCP:api.cloud.local:443" AS #1
20 State% = NSTATUS(1)
30 IF State% = 3 THEN PRINT "Session Established!"
40 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NSTATUS` retrieves the `channel->state` enum from `VNetChannelContext`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NCONNECTED.md`** — Connection boolean.
- **`NERROR.md`** — Socket error code.
