# Universal Peer-to-Peer, BSD Sockets, Event Trapping & Freestanding Security Subsystem

BASIC++ provides a comprehensive, low-latency, event-driven networking stack designed for modern microcontrollers (such as ESP32, RP2040, STM32, ARM Cortex-M), desktop OSes (Windows, Linux, macOS), and embedded Linux gateways.

---

## 1. Architectural Principles

1. **Vendor-Agnostic P2P Architecture (`PEER.*`)**:
   Replaces vendor-locked protocols (such as raw ESP-NOW) with a universal framing protocol operating seamlessly over raw 802.11 action frames, Unix domain datagrams, or UDP broadcast.
2. **Standard BSD Sockets Layer (`SOCK.*`)**:
   Provides non-blocking stream (TCP) and datagram (UDP) multiplexed socket handles with `POLL%`, `BIND`, `LISTEN`, `ACCEPT%`, `SEND`, and `RECV$`.
3. **Event-Driven Trapping & Port Knocking**:
   True interrupt-safe asynchronous event triggers for incoming packets (`ON NET`, `ON PEER`, `ON SNIFF`) and multi-port knocking sequences (`ON PORT(p)`, `PORT.TRIGGER`).
4. **Promiscuous Packet Sniffing & Inspection (`SNIFF.*` / `PACKET.*`)**:
   Zero-copy packet filter enabling Wi-Fi management/beacon frame analysis, raw Ethernet monitoring, RSSI inspection, and source MAC/IP extraction.
5. **Freestanding Cryptography & Authentication (`CRYPTO.*`)**:
   Zero-dependency C17 cryptographic primitives featuring FIPS-compliant SHA-256 hashing, HMAC-SHA256 message authentication, key generation, and authenticated stream encryption.

---

## 2. Language Keywords & Statement Syntax

### Universal Peer-to-Peer Subsystem (`PEER.*`)

| Statement / Command | Syntax | Description |
|---|---|---|
| `PEER.INIT` | `PEER.INIT [channel%]` | Initializes the peer subsystem on the specified wireless/bus channel (default: 1). |
| `PEER.ADD` | `PEER.ADD peer_id$ [, channel%]` | Registers a destination MAC address or node identifier. |
| `PEER.SEND` | `PEER.SEND peer_id$, data$` | Broadcasts or sends an unencrypted/encrypted P2P frame to `peer_id$`. |
| `PEER.RECV` | `PEER.RECV var$` | Retrieves the oldest pending received frame into `var$`. |
| `PEER ON/OFF/STOP` | `PEER ON` / `OFF` / `STOP` | Enables, disables, or temporarily pauses peer packet arrival interrupts. |

### Low-Level BSD Socket Subsystem (`SOCK.*`)

| Function / Statement | Syntax | Return Type | Description |
|---|---|---|---|
| `SOCK.OPEN%` | `SOCK.OPEN%(proto$)` | `INTEGER` | Allocates a socket descriptor (`"TCP"`, `"UDP"`, or `"RAW"`). |
| `SOCK.BIND` | `SOCK.BIND h%, port%` | *Statement* | Binds socket handle `h%` to local port `port%`. |
| `SOCK.LISTEN` | `SOCK.LISTEN h%, backlog%` | *Statement* | Places socket handle `h%` into listening mode. |
| `SOCK.ACCEPT%` | `SOCK.ACCEPT%(listen_h%)` | `INTEGER` | Accepts an incoming connection and returns a client descriptor. |
| `SOCK.SEND` | `SOCK.SEND h%, data$` | *Statement* | Transmits payload data over socket handle `h%`. |
| `SOCK.RECV$` | `SOCK.RECV$(h%, [max_len% [, timeout_ms%]])` | `STRING` | Reads incoming stream or datagram bytes into a string. |
| `SOCK.POLL%` | `SOCK.POLL%(h%, [mask% [, timeout_ms%]])` | `INTEGER` | Polls socket for readable (`1`) or writable (`2`) events. |
| `SOCK.CLOSE` | `SOCK.CLOSE h%` | *Statement* | Closes the socket descriptor and releases buffers. |

### Event Trapping & Port Knocking Subsystem

| Statement | Syntax | Description |
|---|---|---|
| `ON NET GOSUB` | `ON NET GOSUB line_num` | Registers an interrupt handler for generic network packet arrival. |
| `NET ON/OFF/STOP` | `NET ON` / `OFF` / `STOP` | Enables, disables, or suspends `ON NET` event delivery. |
| `ON PEER GOSUB` | `ON PEER GOSUB line_num` | Registers an interrupt handler for P2P connectionless frame arrival. |
| `ON PORT(p) GOSUB`| `ON PORT(port%) GOSUB line_num` | Registers an interrupt handler for incoming activity on port `port%`. |
| `PORT(p) ON/OFF/STOP` | `PORT(port%) ON` / `OFF` / `STOP` | Enables, disables, or suspends port trap triggers on `port%`. |
| `PORT.TRIGGER` | `PORT.TRIGGER host$, p1 [, p2 ...]` | Executes a client-side port knocking sequence across ports `p1`, `p2`, etc. |
| `PORT.FIRE` | `PORT.FIRE port%` | Manually triggers the registered port handler for `port%`. |

### Promiscuous Sniffing & Packet Inspection

| Statement / Function | Syntax | Return Type | Description |
|---|---|---|---|
| `SNIFF.START` | `SNIFF.START [ch%] [, filter$]` | *Statement* | Activates promiscuous packet capture with optional filter. |
| `SNIFF.STOP` | `SNIFF.STOP` | *Statement* | Deactivates promiscuous packet capture. |
| `ON SNIFF GOSUB` | `ON SNIFF GOSUB line_num` | *Statement* | Traps promiscuous packet capture events. |
| `PACKET.MAC$` | `PACKET.MAC$()` | `STRING` | Returns source MAC address of the last captured packet. |
| `PACKET.RSSI%` | `PACKET.RSSI%()` | `INTEGER` | Returns signal strength (dBm) of the last captured packet. |
| `PACKET.PAYLOAD$` | `PACKET.PAYLOAD$()` | `STRING` | Returns raw binary or string payload of the packet. |
| `PACKET.LEN%` | `PACKET.LEN%()` | `INTEGER` | Returns payload byte count. |
| `PACKET.SRC$` | `PACKET.SRC$()` | `STRING` | Returns source IP address string. |
| `PACKET.PORT%` | `PACKET.PORT%()` | `INTEGER` | Returns source TCP/UDP port number. |
| `PACKET.TYPE%` | `PACKET.TYPE%()` | `INTEGER` | Returns 802.11 frame type or IP protocol number. |

### Freestanding Cryptography Subsystem (`CRYPTO.*`)

| Function | Syntax | Return Type | Description |
|---|---|---|---|
| `CRYPTO.KEY$` | `CRYPTO.KEY$([bits%])` | `STRING` | Generates a cryptographically secure random hexadecimal key (default: 256 bits). |
| `CRYPTO.HASH$` | `CRYPTO.HASH$(data$)` | `STRING` | Computes 64-character lowercase hexadecimal SHA-256 digest. |
| `CRYPTO.HMAC$` | `CRYPTO.HMAC$(key$, data$)` | `STRING` | Computes HMAC-SHA256 authentication code for message integrity. |
| `CRYPTO.ENCRYPT$` | `CRYPTO.ENCRYPT$(key$, plain$)` | `STRING` | Encrypts plaintext string into an authenticated hexadecimal ciphertext. |
| `CRYPTO.DECRYPT$` | `CRYPTO.DECRYPT$(key$, cipher$)` | `STRING` | Decrypts ciphertext back to plaintext using the shared key. |

---

## 3. Practical Example Programs

### A. Non-Blocking TCP Echo Server with BSD Sockets

```basic
10 REM BSD Sockets TCP Echo Server
20 H_SRV% = SOCK.OPEN%("TCP")
30 SOCK.BIND H_SRV%, 8080
40 SOCK.LISTEN H_SRV%, 5
50 PRINT "Echo server listening on port 8080..."
60 H_CLI% = SOCK.ACCEPT%(H_SRV%)
70 IF H_CLI% > 0 THEN GOTO 100
80 SLEEP 10
90 GOTO 60
100 MSG$ = SOCK.RECV$(H_CLI%, 1024, 200)
110 IF LEN(MSG$) > 0 THEN SOCK.SEND H_CLI%, "ECHO: " + MSG$
120 SOCK.CLOSE H_CLI%
130 SOCK.CLOSE H_SRV%
140 PRINT "Connection served and closed."
150 END
```

### B. Secure Authenticated IoT Peer-to-Peer Telemetry

```basic
10 REM Universal PEER Telemetry Exchange
20 PEER.INIT 6
30 PEER.ADD "24:0A:C4:11:22:33", 6
40 SECRET_KEY$ = "A9F8E7D6C5B4A3920192837465564738"
50 RAW_PAYLOAD$ = NET.PACK$("TEMP: 22.4 C, HUM: 45%")
60 CIPHER$ = CRYPTO.ENCRYPT$(SECRET_KEY$, RAW_PAYLOAD$)
70 MAC$ = CRYPTO.HMAC$(SECRET_KEY$, CIPHER$)
80 PEER.SEND "24:0A:C4:11:22:33", CIPHER$ + "|" + MAC$
90 PRINT "Encrypted & authenticated frame dispatched."
100 END
```
