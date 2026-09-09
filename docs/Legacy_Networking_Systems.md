<!--
Title:        Legacy_Networking_Systems
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/fujinet.c, engine/src/runtime/tnfs.c, engine/src/runtime/gemini.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Legacy Networking Systems: Gemini, Gopher, TNFS & FujiNet

The authoritative technical specification and reference manual for retro-computing network protocols, virtual peripheral adapters, and remote filesystem streaming in BASIC++.

---

## 1. Architectural Overview & Virtual Device Integration

BASIC++ integrates retro-modern communication protocols directly into its unified virtual peripheral bus:
- **Atari FujiNet Peripheral Bridge (`N:`)**: Emulates the Atari SIO-based FujiNet multi-peripheral adapter (`engine/src/device/fujinet.c`), routing device paths to native network sockets.
- **Trusted Network File System (TNFS)**: Implements the UDP-based TNFS client protocol (`engine/src/runtime/tnfs.c` on UDP port 9868) for remote disk mounting and remote file execution.
- **Lightweight Hypertext Protocols**: Integrates Project Gemini (`engine/src/runtime/gemini.c`) and RFC 1436 Gopher (`engine/src/runtime/gopher.c`) for alternative internet retrieval without bloated modern web engines.

---

## 2. Atari 8-Bit FujiNet Emulation (`N:`)

The FujiNet peripheral driver intercepts the `N:` device prefix in standard BASIC file statements, converting device paths into live TCP, UDP, or HTTP network streams:

```basic
10 REM Open live TCP socket stream via FujiNet device prefix
20 OPEN "N:tcp://retro.bbs.org:23/" FOR RANDOM AS #1
30 PRINT #1, "HELLO BBS"
40 LINE INPUT #1, Response$
50 PRINT "BBS Response: "; Response$
60 CLOSE #1
```

### Peripheral Status Queries

Programs can query the state of the virtual FujiNet adapter using `FUJI.STATUS$()`:
```basic
10 Status$ = FUJI.STATUS$()
20 PRINT "FujiNet Adapter Status: "; Status$
```

---

## 3. Trusted Network File System (TNFS)

TNFS allows BASIC++ programs to mount remote archives (such as `tnfs.fujinet.online`) over UDP port 9868 as virtual drives:

```basic
10 REM Mount remote TNFS archive to virtual drive slot
20 OPEN "N:tnfs://tnfs.fujinet.online/games" FOR INPUT AS #1
30 REM Browse remote directory contents
40 WHILE NOT EOF(1)
50   LINE INPUT #1, Entry$
60   PRINT "Remote File: "; Entry$
70 WEND
80 CLOSE #1
```

---

## 4. Project Gemini & RFC 1436 Gopher Protocols

BASIC++ provides built-in intrinsic functions and statements for querying lightweight capsules:

### Intrinsic Protocol Functions

- **`GEMINI.GET$(url$)`**: Fetches a Gemtext capsule document over TLS/TCP.
- **`GEMINI.STATUS%()`**: Returns the integer HTTP/Gemini status code of the last transaction (e.g. 20 for Success).
- **`GEMINI.META$()`**: Returns response metadata or MIME type (e.g. `"text/gemini"`).
- **`GOPHER.GET$(url$)`**: Retrieves a plaintext or directory menu document from an RFC 1436 Gopher server.

### Protocol Retrieval Example

```basic
10 Doc$ = GEMINI.GET$("gemini://gemini.circumlunar.space/")
20 IF GEMINI.STATUS%() = 20 THEN
30   PRINT "Document Content:"
40   PRINT Doc$
50 ELSE
60   PRINT "Gemini Error: Code "; GEMINI.STATUS%(); " ("; GEMINI.META$(); ")"
70 END IF
```
