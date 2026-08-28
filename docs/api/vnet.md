# C17 API Reference: Virtual Network Sockets (`runtime/vnet.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual Network Sockets Subsystem (`runtime/vnet.h`, implemented in `engine/src/runtime/vnet.c`) provides non-blocking TCP/UDP socket streaming, server listening, client connections, HTTP request queries, and virtual network device bindings (`N:` device) for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Multi-Channel Network Stack**: Manages up to 16 concurrent network socket channels (`VNET_MAX_CHANNELS`).
- **Client & Server Operations**:
  - Outbound Client Sockets: `vnet_open()` connecting to remote TCP/UDP endpoints.
  - Inbound Listening Servers: `vnet_open_host()` and `vnet_accept()` accepting incoming connections.
- **Data Streaming & Framing**: Non-blocking packet transmission (`vnet_send()`) and reception (`vnet_recv()`).
- **VDev Integration**: Bridges raw socket channels into `VDev` descriptors (`vnet_create_vdev()`), allowing standard BASIC stream statements (`OPEN "TCP://host:port" FOR RANDOM AS #1`, `GET#`, `PUT#`).

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/vnet.h"
#include "device/vdev.h"
#include "memory/memory.h"
```

## 3. Data Structures & Types

```c
#define VNET_MAX_CHANNELS 16

/* Opaque Handle to Virtual Network Context */
typedef struct VNetContext VNetContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle
```c
VNetContext *vnet_init(MemoryContext *mem);
void         vnet_shutdown(VNetContext *ctx);
```

### Connection Management
```c
/**
 * @brief Connects an outbound client socket to a remote host.
 */
BppError vnet_open(VNetContext *ctx, int channel, const char *protocol, const char *host, int port);

/**
 * @brief Binds and listens on a local port for incoming connections.
 */
BppError vnet_open_host(VNetContext *ctx, int channel, int port);

/**
 * @brief Accepts an incoming connection from a listening channel.
 */
BppError vnet_accept(VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len);

/**
 * @brief Closes an active network channel.
 */
void vnet_close(VNetContext *ctx, int channel);
```

### Data Transmission & Status
```c
BppError    vnet_send(VNetContext *ctx, int channel, const char *data, size_t len);
BppError    vnet_recv(VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len);
int         vnet_status(VNetContext *ctx, int channel);
bool        vnet_connected(VNetContext *ctx, int channel);
const char *vnet_address(VNetContext *ctx, int channel);
int         vnet_http_status(VNetContext *ctx, int channel);

VDev        vnet_create_vdev(VNetContext *ctx, const char *name, const char *protocol, const char *host, int port);
```

## 5. Architectural Invariants

- **Capability Verification**: Network socket operations require `CAP_NET` in `SecurityContext`.
- **Channel Bounds**: Channel numbers are validated between 1 and `VNET_MAX_CHANNELS` (16).

## 6. Code Example: Connecting to a TCP Server in C

```c
#include "runtime/vnet.h"

void fetch_time_data(VNetContext *vnet) {
    BppError err = vnet_open(vnet, 1, "TCP", "time.nist.gov", 13);
    if (err.code == 0) {
        char buf[128] = {0};
        size_t bytes_read = 0;
        vnet_recv(vnet, 1, buf, sizeof(buf) - 1, &bytes_read);
        vnet_close(vnet, 1);
    }
}
```
