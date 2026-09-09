// FILENAME: vnet_nat.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libserver (vnet.c, vnet_nat.c, stmt_upnp.c, func_upnp.c, vdev_upnp.c)
// Declares UPnP IGD, NAT-PMP, and pluggable network provider interfaces.

#ifndef BASICPP_VNET_NAT_H
#define BASICPP_VNET_NAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// ---- NAT Status Codes ----
//

typedef enum {
    NAT_STATUS_OK             = 0,
    NAT_STATUS_NOT_FOUND      = 1,
    NAT_STATUS_DISCOVERY_ERR  = 2,
    NAT_STATUS_SOAP_ERR       = 3,
    NAT_STATUS_CONFLICT       = 4,
    NAT_STATUS_UNSUPPORTED    = 5,
    NAT_STATUS_OFFLINE_SIM    = 6
} BppNatStatus;

//
// ---- Pluggable Provider Interface ----
//

typedef struct VNetNatProvider {
    const char   *name;
    BppNatStatus (*discover)(void *userdata, int timeout_ms);
    BppNatStatus (*get_external_ip)(void *userdata, char *out_ip, size_t max_len);
    BppNatStatus (*add_mapping)(void *userdata, int ext_port, int int_port, const char *proto, const char *desc, int lease_seconds);
    BppNatStatus (*delete_mapping)(void *userdata, int ext_port, const char *proto);
    BppNatStatus (*query_status)(void *userdata, int ext_port, const char *proto, char *out_info, size_t max_len);
    void         *userdata;
} VNetNatProvider;

//
// ---- Public API Functions ----
//

void         vnet_nat_init(void);
void         vnet_nat_shutdown(void);
BppNatStatus vnet_nat_discover(int timeout_ms);
BppNatStatus vnet_nat_forward(int ext_port, int int_port, const char *proto, const char *desc, int lease_seconds);
BppNatStatus vnet_nat_unforward(int ext_port, const char *proto);
BppNatStatus vnet_nat_get_external_ip(char *out_ip, size_t max_len);
BppNatStatus vnet_nat_get_status(int ext_port, const char *proto, char *out_info, size_t max_len);
void         vnet_nat_set_provider(const VNetNatProvider *provider);
void         vnet_nat_reset_default_provider(void);
void         vnet_nat_enable_simulation(bool enable);

#ifdef __cplusplus
}
#endif

#endif // BASICPP_VNET_NAT_H
