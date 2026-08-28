# `vnet` Virtual Network Sockets Subsystem (`libserver`)

## 1. Architectural Purpose & Overview

The `vnet` subsystem (`engine/src/server/vnet.c`) provides non-blocking TCP/UDP socket communication, network channel bindings, and server listening capabilities.

### Key Architectural Invariants:
- **Channel Integration**: Sockets open as standard BASIC channels (`OPEN "N:TCP://host:port" AS #1`).
- **Non-Blocking I/O**: Supports event-driven socket polling without stalling the VM execution loop.
- **Security Check**: Enforced via `CAP_NET` capability flags.

---

## 2. Technical API Signatures (C17)

```c
VNetSocket *vnet_connect(const char *host, int port, int protocol);
VNetSocket *vnet_listen(int port, int backlog);
int vnet_send(VNetSocket *sock, const void *buf, size_t len);
int vnet_recv(VNetSocket *sock, void *buf, size_t max_len);
void vnet_close(VNetSocket *sock);
```
