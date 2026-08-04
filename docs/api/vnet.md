# Virtual Networking API Reference

Header File: [`include/vnet.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/vnet.h)

## Overview
Supports standard TCP/IP networking, sockets client listener connections.

## Exposed API Entities
### Structs & Types
- `VNetContext VNetContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vnet_shutdown` | `void` | `VNetContext *ctx` |
| `vnet_open` | `BppError` | `VNetContext *ctx, int channel, const char *protocol, const char *host, int port` |
| `vnet_open_host` | `BppError` | `VNetContext *ctx, int channel, int port` |
| `vnet_accept` | `BppError` | `VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len` |
| `vnet_send` | `BppError` | `VNetContext *ctx, int channel, const char *data, size_t len` |
| `vnet_recv` | `BppError` | `VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len` |
| `vnet_close` | `void` | `VNetContext *ctx, int channel` |
| `vnet_status` | `int` | `VNetContext *ctx, int channel` |
| `vnet_connected` | `bool` | `VNetContext *ctx, int channel` |
| `vnet_http_status` | `int` | `VNetContext *ctx, int channel` |
| `vnet_create_vdev` | `VDev` | `VNetContext *ctx, const char *name, const char *protocol, const char *host, int port` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "vnet.h"

void net_open(VMContext *vm) {
    vnet_open_socket(vm, "TCP:google.com:80");
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
