# Networking in BASIC++

BASIC++ supports seamless TCP and UDP networking using the Virtual Device (VDev2) abstraction layer.

## How to use

You can open a network socket exactly like a file, using the `OPEN` command with `TCP:` or `UDP:` prefixed URIs.

### Syntax
`OPEN "TCP:host:port" FOR mode AS #chan`
`OPEN "UDP:host:port" FOR mode AS #chan`

### Examples
```basic
10 OPEN "TCP:example.com:80" FOR OUTPUT AS #1
20 PRINT #1, "GET / HTTP/1.1"
30 PRINT #1, "Host: example.com"
40 PRINT #1, ""
50 INPUT #1, RESPONSE$
60 PRINT RESPONSE$
70 CLOSE #1
```

## Supported Protocols
- `TCP`: Stream-oriented connections.
- `UDP`: Datagram-oriented connections.

The networking subsystem is fully integrated with BASIC++ file I/O operations (`PRINT #`, `INPUT #`, `GET`, `PUT`), making it extremely simple to communicate over the network without needing new keywords or complex socket APIs.
