// FILENAME: vnet.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, vm_internal.h)
// NEEDED BY: libkernel (fujinet.c)
// NEEDED BY: libserver (vnet.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides core logic and interface definitions for vnet within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_VNET_H
#define RUNTIME_VNET_H

#include <stdbool.h>
#include <stddef.h>
#include "types/types.h"
#include "memory/memory.h"
#include "device/vdev.h"

#define VNET_MAX_CHANNELS 16

typedef struct VNetContext VNetContext;

VNetContext *vnet_init(MemoryContext *mem);
void         vnet_shutdown(VNetContext *ctx);

// Core network socket actions
BppError vnet_open(VNetContext *ctx, int channel, const char *protocol, const char *host, int port);
BppError vnet_open_host(VNetContext *ctx, int channel, int port);
BppError vnet_accept(VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len);
BppError vnet_send(VNetContext *ctx, int channel, const char *data, size_t len);
BppError vnet_recv(VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len);
void     vnet_close(VNetContext *ctx, int channel);

// Network status functions
int  vnet_status(VNetContext *ctx, int channel);
bool vnet_connected(VNetContext *ctx, int channel);
const char *vnet_address(VNetContext *ctx, int channel);
int  vnet_http_status(VNetContext *ctx, int channel);

// VDev interface registration wrapper
VDev vnet_create_vdev(VNetContext *ctx, const char *name, const char *protocol, const char *host, int port);

#endif // RUNTIME_VNET_H
