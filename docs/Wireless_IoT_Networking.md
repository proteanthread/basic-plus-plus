# Wireless & IoT Networking Reference

BASIC++ provides comprehensive support for microcontroller and IoT wireless capabilities across Wi-Fi, ESP-NOW, Bluetooth Classic (SPP), BLE, and NFC/RFID.

## 1. Wi-Fi SoftAP, Scanning & Promiscuous Sniffer
- `WIFI.AP ssid$, password$ [, channel [, max_connections]]`: Initializes the onboard Wi-Fi chip into Soft Access Point (AP) mode.
- `WIFI.SCAN`: Scans surrounding BSSIDs and prints detected SSIDs, RSSI signal levels, and encryption channels.
- `WIFI.SNIFF ON [, channel]`: Enables raw 802.11 packet sniffer in promiscuous mode.
- `WIFI.SNIFF OFF`: Disables packet sniffing.

## 2. ESP-NOW Zero-Latency P2P Protocol
- `ESPNOW.INIT [channel]`: Initializes the ESP-NOW peer-to-peer MAC protocol.
- `ESPNOW.ADD.PEER mac$ [, channel]`: Registers a remote peer MAC address.
- `ESPNOW.SEND mac$, data$`: Transmits a zero-latency payload to the target peer.
- `ESPNOW.RECV var$`: Receives an incoming packet into a variable.

## 3. Bluetooth Classic SPP & Bluetooth Low Energy (BLE)
- `BT.START name$`: Starts Bluetooth Classic Serial Port Profile (SPP).
- `BT.CONNECT mac_or_name$`: Initiates an outgoing SPP connection to a remote Bluetooth device.
- `BLE.ADV.START name$, uuid$`: Starts advertising a BLE GATT service beacon.
- `BLE.ADV.STOP`: Stops BLE beacon advertising.
- `BLE.SCAN [duration_ms]`: Scans for nearby BLE advertisements and RSSI strengths.

## 4. NFC & RFID Peripheral Subsystem (PN532 / MFRC522)
- `NFC.INIT [i2c_addr]`: Initializes external NFC/RFID reader.
- `NFC.SCAN card_uid$`: Scans for passive ISO14443A cards and retrieves the UID hex string.
- `NFC.READ block_num, var$`: Reads a 16-byte memory block from an authenticated card.
- `NFC.WRITE block_num, data$`: Writes data to the target card memory block.
- `NFC.EMULATE uid$`: Configures NFC chip to emulate a passive card with a specified UID.
