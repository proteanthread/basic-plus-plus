# BASIC++ v6.5.2 FujiNet Integration

## 1. WHAT IS FujiNet

FujiNet is a network adapter designed for Atari 8-bit computers that provides WiFi connectivity, virtual disk drives, and network I/O through the Atari SIO (Serial I/O) bus. BASIC++ emulates the FujiNet device interface, allowing programs written for FujiNet-equipped Atari systems to run on modern hardware.

The FujiNet emulation is implemented in engine/src/device/fujinet.c and is part of the libhardware library.

## 2. THE N: DEVICE

FujiNet uses the N: device prefix for network operations. BASIC++ routes N: device paths through the VFS to the VNet subsystem:

```basic
10 OPEN "N:tcp://example.com:80/" FOR INPUT AS #1
20 INPUT #1, Response$
30 PRINT Response$
40 CLOSE #1
```

The N: prefix is recognized by the VFS as a FujiNet network device path. The remaining path is parsed as a URL with protocol, host, port, and path components.

## 3. SUPPORTED PROTOCOLS

| URL Prefix | Protocol | Description |
|-----------|----------|-------------|
| N:tcp:// | TCP | Stream socket connection |
| N:udp:// | UDP | Datagram socket |
| N:http:// | HTTP | HTTP client request |
| N:https:// | HTTPS | HTTPS client request |
| N:tnfs:// | TNFS | Trivial Network File System |

## 4. HTTP REQUESTS

FujiNet-style HTTP requests:

```basic
10 OPEN "N:http://api.example.com/data" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30   LINE INPUT #1, L$
40   PRINT L$
50 WEND
60 CLOSE #1
```

POST requests use the OUTPUT mode:

```basic
10 OPEN "N:http://api.example.com/submit" FOR OUTPUT AS #1
20 PRINT #1, "name=Alice&score=95"
30 CLOSE #1
```

## 5. TNFS (TRIVIAL NETWORK FILE SYSTEM)

TNFS provides remote file access over UDP. Programs can read and write files on a TNFS server:

```basic
10 OPEN "N:tnfs://server.local/data.txt" FOR INPUT AS #1
20 LINE INPUT #1, Data$
30 CLOSE #1
```

TNFS is commonly used in the retro computing community for sharing disk images and program files.

## 6. DEVICE STATUS

FujiNet device status is available through the standard device discovery API:

```basic
> DEVICES
Slot  Type      Name          Status
  6   FujiNet   FujiNet       Idle
```

The FujiNet device appears in the device list when the libhardware library is linked. Its status changes to Active when a N: path is opened.

## 7. ATARI COMPATIBILITY

Programs written for Atari BASIC with FujiNet extensions can run on BASIC++ with the Atari dialect (APPL or a custom Atari configuration). The N: device prefix, XIO commands, and STATUS operations are translated to BASIC++ VNet operations internally.

## 8. SECURITY

FujiNet network operations are subject to the same security level restrictions as all VNet operations. At security levels 3 (EDUCATIONAL) and above, all N: device operations are denied.
