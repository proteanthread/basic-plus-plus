/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file vnet.h
 * @brief Runtime component implementation and public API surface for vnet.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for vnet.h within the runtime subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file vnet.h
 * @brief Virtual Network Stack & Sockets (VNet) interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares socket-backed channel management and statement handlers.
 * - Why it exists: Emulates internet/intranet TCP/UDP connectivity in standard BASIC++.
 * - Why it works this way: It manages an array of active socket descriptors mapped
 *   to virtual network channels.
 */

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

#endif /* RUNTIME_VNET_H */
