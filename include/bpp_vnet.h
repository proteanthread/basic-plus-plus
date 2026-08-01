/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_vnet.h
 * @brief Virtual Network Stack & Sockets (VNet) interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares socket-backed channel management and statement handlers.
 * - Why it exists: Emulates internet/intranet TCP/UDP connectivity in standard BASIC++.
 * - Why it works this way: It manages an array of active socket descriptors mapped
 *   to virtual network channels.
 */

#ifndef BPP_VNET_H
#define BPP_VNET_H

#include <stdbool.h>
#include <stddef.h>
#include "bpp_types.h"
#include "bpp_memory.h"
#include "bpp_vdev.h"

#define VNET_MAX_CHANNELS 16

typedef struct VNetContext VNetContext;

VNetContext *vnet_init(MemoryContext *mem);
void         vnet_shutdown(VNetContext *ctx);

/* Core network socket actions */
BppError vnet_open(VNetContext *ctx, int channel, const char *protocol, const char *host, int port);
BppError vnet_open_host(VNetContext *ctx, int channel, int port);
BppError vnet_accept(VNetContext *ctx, int listen_channel, int client_channel, char *client_ip_buf, int ip_buf_len);
BppError vnet_send(VNetContext *ctx, int channel, const char *data, size_t len);
BppError vnet_recv(VNetContext *ctx, int channel, char *buf, size_t max_len, size_t *out_len);
void     vnet_close(VNetContext *ctx, int channel);

/* Network status functions */
int  vnet_status(VNetContext *ctx, int channel);
bool vnet_connected(VNetContext *ctx, int channel);
const char *vnet_address(VNetContext *ctx, int channel);
int  vnet_http_status(VNetContext *ctx, int channel);

/* VDev interface registration wrapper */
VDev vnet_create_vdev(VNetContext *ctx, const char *name, const char *protocol, const char *host, int port);

#endif /* BPP_VNET_H */
