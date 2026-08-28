# NCONNECTED Function Reference

The `NCONNECTED` built-in function returns a boolean status flag indicating whether a network socket I/O channel is currently connected to a remote host.

## Syntax

```basic
is_connected% = NCONNECTED(channel%)
```

## Parameters

- **`channel%`** — Active network socket channel number (`#1` to `#16`).

## Return Value

- Returns **`-1`** (true) if the network socket is established and actively connected.
- Returns **`0`** (false) if the connection is closed, disconnected, or resetting.

---

## Code Examples

### Example 1: Polling Connection State
```basic
10 OPEN "TCP:game.server.local:7777" AS #1
20 IF NCONNECTED(1) THEN
30     PRINT "Connected to Game Server!"
40     NWRITE 1, "HELLO" + CHR$(10)
50 ELSE
60     PRINT "Connection Failed!"
70 END IF
80 CLOSE #1
```

---

## Engine Implementation (`server/vnet.c`)

In `engine/src/server/vnet.c`:
`NCONNECTED` inspects the socket connection state flags in `VNetChannelContext`.

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 52 | Bad File Number (`ERR_BAD_FILE_NUMBER`) | Channel not open |

---

## Cross-References

- **`NEOF.md`** — End-of-stream detection.
- **`NSTATUS.md`** — Socket error state code.
- **`NWRITE.md`** — Socket write.
