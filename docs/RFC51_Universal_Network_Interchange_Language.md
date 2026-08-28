# BASIC++ RFC 51 Universal Network Interchange Language (NIL) Architecture

## Overview
BASIC++ v6.5.2 implements the foundational principles of **RFC 51 ("Proposal for a Network Interchange Language")** by M. Elie (May 1970). It provides a unified data serialization, payload compression, and distributed remote execution layer operating identically across all host binaries (`baspp.exe`, `bpp.exe`, `bs.exe`) and microcontroller IoT targets (`iot.exe` / ESP32).

---

## 1. RFC 51 Stream Compression Engine (`COMSTRING` & `COMPVECTOR`)
Provides fast, zero-allocation run-length and tokenized whitespace compression to optimize low-bandwidth networks, low-MTU wireless links (BLE 20-byte MTU, ESP-NOW 250-byte frames), and NFC sectors (16 bytes).

### Functions:
- **`COMSTR$(raw$)`** / **`NIL.COMPRESS$(raw$)`**: Encodes repeating characters and spaces into compact RFC 51 control sequences.
- **`DECOMSTR$(comp$)`** / **`NIL.DECOMPRESS$(comp$)`**: Decodes compressed strings back to their original representation with 100% roundtrip fidelity.

---

## 2. Universal Bead & PLEX Marshaling (RFC 51 Data Definition)
Serializes numeric, string, array (`VECTOR`), and object/map (`PLEX`) data types into canonical, endian-neutral network byte order with 1-byte identification and parameter headers.

### Functions & Statements:
- **`NET.PACK$(expr_or_array)`** / **`NIL.PACK$(...)`**: Serializes any scalar, array, or object into a binary network string.
- **`NET.UNPACK packet$, dest_var`** / **`NIL.UNPACK ...`**: Deserializes a network packet into the target variable or array.

### Direct Transport Integration:
```basic
REM Broadcast structured state over ESP-NOW
ESPNOW.SEND "FF:FF:FF:FF:FF:FF", NET.PACK$("NODE_ONLINE")

REM Publish compressed sensor telemetry over MQTT
MQTT.PUB "sensors/hall", COMSTR$("HALL: 142.8")

REM Write structured object to NFC tag block
NFC.WRITE 4, NET.PACK$("SECURITY_TOKEN_99")
```

---

## 3. Distributed Remote Execution & RPC (RFC 51 Control Sublanguage)
Specifies *WHERE* and *WHEN* computations occur, allowing host machines to dispatch statement execution and function evaluation directly to edge IoT devices.

### Statements & Functions:
- **`REMOTE.EXEC target_uri$, statement$`**: Sends and executes a statement on the remote node.
- **`REMOTE.EVAL$(target_uri$, expression$)`**: Evaluates an expression on the remote node and returns the string result.
- **`IOT.RPC$(target_uri$, func_call$)`**: Executes a remote procedure call and returns the response.
- **`OPEN "REMOTE:<uri>" FOR RANDOM AS #filenum`**: Opens a bidirectional stream to the remote node.

### Supported URI Transports:
- **`COM<n>` / `/dev/tty*`**: Physical USB-UART Serial Bridge.
- **`TCP://<host>:<port>`** / **`UDP://<host>:<port>`**: Network sockets.
- **`ESPNOW://<mac>`**: ESP-NOW peer-to-peer wireless.
- **`BT://<name>`** / **`BLE://<uuid>`**: Bluetooth Classic SPP and BLE GATT.
- **`NFC://<block>`**: Near Field Communication / RFID tag interface.
- **`MQTT://<broker>/<topic>`**: Lightweight pub/sub messaging queue.
- **`N:<uri>`**: FujiNet remote device bus & TNFS remote file system.
- **`IPC://<pipe>`**: Local inter-process communication pipe.
