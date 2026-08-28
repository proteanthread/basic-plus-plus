# `fujinet` FujiNet Network Device Emulation (`libhardware`)

## 1. Architectural Purpose & Overview

The `fujinet` subsystem (`engine/src/device/fujinet.c`) emulates FujiNet multi-peripheral hardware adapters, providing network disk emulation, TCP/UDP sockets, and modern cloud storage access for retro environments.

### Key Architectural Invariants:
- **Network Devices**: Emulates `N:` network devices and TNFS remote disk mounting.
- **Protocol Handlers**: Bridges TCP, UDP, HTTP, and WebSocket connections directly into BASIC channel I/O.

---

## 2. Technical API Signatures (C17)

```c
void fujinet_init(void);
int fujinet_mount_disk(int drive_no, const char *url);
int fujinet_open_socket(const char *host, int port, int protocol);
```
