<!--
Title:        Universal_Networking_Sockets_Security
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/vnet.c, engine/src/platform/plat_net.c
Generated:    no, hand-written
Status:       current
-->

# Universal Peer-to-Peer, BSD Sockets, Event Trapping & Wireless IoT Subsystem

BASIC++ provides a comprehensive, low-latency, event-driven networking stack designed for modern microcontrollers (ESP32, RP2040, ARM Cortex-M), desktop operating systems (Windows, Linux, macOS), and embedded gateways.

---

## 1. Architectural Principles

1. **Vendor-Agnostic P2P Architecture (`PEER.*`)**: Replaces vendor-locked protocols (such as raw ESP-NOW) with a universal framing protocol operating seamlessly across 802.11 action frames, Unix domain datagrams, or UDP broadcast.
2. **Standard BSD Sockets Layer (`SOCK.*`)**: Provides non-blocking stream (TCP) and datagram (UDP) multiplexed socket handles with `POLL%`, `BIND`, `LISTEN`, `ACCEPT%`, `SEND`, and `RECV$`.
3. **Event-Driven Trapping & Port Knocking**: True asynchronous event triggers for incoming packets (`ON NET`, `ON PEER`, `ON SNIFF`) and multi-port knocking sequences (`ON PORT(p)`, `PORT.TRIGGER`).
4. **Promiscuous Packet Sniffing & Inspection (`SNIFF.*` / `PACKET.*`)**: Zero-copy packet filter enabling Wi-Fi management/beacon frame analysis, raw Ethernet monitoring, RSSI inspection, and source MAC/IP extraction.
5. **Physical Wireless & IoT Radios**: Integrated statements for Wi-Fi SoftAP mode, Bluetooth Classic SPP, Bluetooth Low Energy (BLE) beacons, and NFC/RFID card communication.

---

## 2. Universal Peer-to-Peer Subsystem (`PEER.*`)

| Statement / Command | Syntax | Description |
|:---|:---|:---|
| `PEER.INIT` | `PEER.INIT [channel%]` | Initializes the peer subsystem on the specified wireless/bus channel. |
| `PEER.ADD` | `PEER.ADD peer_id$ [, channel%]` | Registers a destination MAC address or node identifier. |
| `PEER.SEND` | `PEER.SEND peer_id$, data$` | Broadcasts or transmits an authenticated payload to `peer_id$`. |
| `PEER.RECV` | `PEER.RECV var$` | Retrieves the oldest pending received frame into `var$`. |
| `PEER ON/OFF/STOP` | `PEER ON` / `OFF` / `STOP` | Enables, disables, or pauses packet arrival interrupts. |

```basic
10 PEER.INIT 1
20 PEER.ADD "FF:FF:FF:FF:FF:FF"    ' Register broadcast MAC address
30 PEER.SEND "FF:FF:FF:FF:FF:FF", "PING_BEACON"
40 PEER ON                         ' Enable event interrupts
```

---

## 3. Low-Level BSD Socket Subsystem (`SOCK.*`)

| Function / Statement | Syntax | Return Type | Description |
|:---|:---|:---|:---|
| `SOCK.OPEN%` | `SOCK.OPEN%(proto$)` | `INTEGER` | Allocates a socket handle (`"TCP"`, `"UDP"`, or `"RAW"`). |
| `SOCK.BIND` | `SOCK.BIND h%, port%` | *Statement* | Binds socket handle `h%` to local port `port%`. |
| `SOCK.LISTEN` | `SOCK.LISTEN h%, backlog%` | *Statement* | Places socket handle `h%` into listening state. |
| `SOCK.ACCEPT%` | `SOCK.ACCEPT%(listen_h%)` | `INTEGER` | Accepts an incoming connection, returning client handle. |
| `SOCK.SEND` | `SOCK.SEND h%, data$` | *Statement* | Transmits payload data over socket handle `h%`. |
| `SOCK.RECV$` | `SOCK.RECV$(h%, max_bytes%)`| `STRING` | Reads pending incoming bytes from socket handle `h%`. |
| `SOCK.CLOSE` | `SOCK.CLOSE h%` | *Statement* | Closes the specified socket descriptor. |

---

## 4. Physical Wireless & IoT Radios

### Wi-Fi SoftAP & Scanning
- `WIFI.AP ssid$, password$ [, channel [, max_conn]]`: Configures chip into Soft Access Point mode.
- `WIFI.SCAN`: Scans surrounding BSSIDs and prints detected SSIDs, RSSI levels, and encryption.
- `WIFI.SNIFF ON [, channel]`: Enables raw 802.11 packet sniffer in promiscuous mode.
- `WIFI.SNIFF OFF`: Disables packet sniffing.

### Bluetooth Classic & BLE Beacons
- `BT.START name$`: Initializes Bluetooth Classic Serial Port Profile (SPP).
- `BT.CONNECT mac_or_name$`: Initiates outgoing SPP connection to a remote Bluetooth device.
- `BLE.ADV.START name$, uuid$`: Advertises a BLE GATT service beacon.
- `BLE.ADV.STOP`: Stops BLE beacon advertising.

### NFC & RFID Peripheral Subsystem
- `NFC.INIT [i2c_addr]`: Initializes external NFC/RFID reader.
- `NFC.SCAN card_uid$`: Scans for passive ISO14443A cards and retrieves the UID hex string.
- `NFC.READ block_num, var$`: Reads a 16-byte memory block from an authenticated card.
- `NFC.WRITE block_num, data$`: Writes data to the target card memory block.
