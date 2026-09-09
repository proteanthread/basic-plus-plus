// FILENAME: vnet_nat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libserver (vnet_nat.h, stmt_upnp.c, func_upnp.c, vdev_upnp.c)
// NEEDS: libplatform (plat_net.h), libcore (memops.h, strops.h)
// Implements UPnP IGD, NAT-PMP, and pluggable network provider subsystems.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/format/snprintf.h"

#include "runtime/vnet_nat.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"

#define MAX_SIM_MAPPINGS 64

typedef struct {
    int  ext_port;
    int  int_port;
    char proto[8];
    char desc[64];
    bool active;
} SimMapping;

static bool g_simulation_mode = false;
static char g_sim_external_ip[64] = "198.51.100.42";
static SimMapping g_sim_mappings[MAX_SIM_MAPPINGS];

//
// ---- Default Simulated / Loopback Provider ----
//

static BppNatStatus sim_discover(void *userdata, int timeout_ms) {
    (void)userdata;
    (void)timeout_ms;
    return NAT_STATUS_OK;
}

static BppNatStatus sim_get_external_ip(void *userdata, char *out_ip, size_t max_len) {
    (void)userdata;
    if (!out_ip || max_len < 8) return NAT_STATUS_NOT_FOUND;
    runtime_strncpy(out_ip, g_sim_external_ip, max_len - 1);
    out_ip[max_len - 1] = '\0';
    return NAT_STATUS_OK;
}

static BppNatStatus sim_add_mapping(void *userdata, int ext_port, int int_port, const char *proto, const char *desc, int lease) {
    (void)userdata;
    (void)lease;
    const char *pr = proto ? proto : "TCP";
    const char *ds = desc ? desc : "BASIC++ Port Forward";

    // Update existing or find runtime_free slot
    int free_slot = -1;
    for (int i = 0; i < MAX_SIM_MAPPINGS; ++i) {
        if (g_sim_mappings[i].active && g_sim_mappings[i].ext_port == ext_port &&
            runtime_strcasecmp(g_sim_mappings[i].proto, pr) == 0) {
            g_sim_mappings[i].int_port = int_port;
            runtime_strncpy(g_sim_mappings[i].desc, ds, sizeof(g_sim_mappings[i].desc) - 1);
            return NAT_STATUS_OK;
        }
        if (!g_sim_mappings[i].active && free_slot < 0) {
            free_slot = i;
        }
    }

    if (free_slot >= 0) {
        g_sim_mappings[free_slot].ext_port = ext_port;
        g_sim_mappings[free_slot].int_port = int_port;
        runtime_strncpy(g_sim_mappings[free_slot].proto, pr, sizeof(g_sim_mappings[free_slot].proto) - 1);
        runtime_strncpy(g_sim_mappings[free_slot].desc, ds, sizeof(g_sim_mappings[free_slot].desc) - 1);
        g_sim_mappings[free_slot].active = true;
        return NAT_STATUS_OK;
    }

    return NAT_STATUS_CONFLICT;
}

static BppNatStatus sim_delete_mapping(void *userdata, int ext_port, const char *proto) {
    (void)userdata;
    const char *pr = proto ? proto : "TCP";
    for (int i = 0; i < MAX_SIM_MAPPINGS; ++i) {
        if (g_sim_mappings[i].active && g_sim_mappings[i].ext_port == ext_port &&
            runtime_strcasecmp(g_sim_mappings[i].proto, pr) == 0) {
            g_sim_mappings[i].active = false;
            return NAT_STATUS_OK;
        }
    }
    return NAT_STATUS_NOT_FOUND;
}

static BppNatStatus sim_query_status(void *userdata, int ext_port, const char *proto, char *out_info, size_t max_len) {
    (void)userdata;
    const char *pr = proto ? proto : "TCP";
    for (int i = 0; i < MAX_SIM_MAPPINGS; ++i) {
        if (g_sim_mappings[i].active && g_sim_mappings[i].ext_port == ext_port &&
            runtime_strcasecmp(g_sim_mappings[i].proto, pr) == 0) {
            if (out_info && max_len > 0) {
                runtime_snprintf(out_info, max_len, "MAPPED %s %d->%d (%s)",
                         g_sim_mappings[i].proto, g_sim_mappings[i].ext_port,
                         g_sim_mappings[i].int_port, g_sim_mappings[i].desc);
            }
            return NAT_STATUS_OK;
        }
    }
    if (out_info && max_len > 0) {
        runtime_snprintf(out_info, max_len, "UNMAPPED %s %d", pr, ext_port);
    }
    return NAT_STATUS_NOT_FOUND;
}

static VNetNatProvider g_sim_provider = {
    .name = "Simulated Loopback NAT Provider",
    .discover = sim_discover,
    .get_external_ip = sim_get_external_ip,
    .add_mapping = sim_add_mapping,
    .delete_mapping = sim_delete_mapping,
    .query_status = sim_query_status,
    .userdata = NULL
};

static const VNetNatProvider *g_active_provider = &g_sim_provider;

//
// ---- Public API Subsystem Functions ----
//

void vnet_nat_init(void) {
    runtime_memset(g_sim_mappings, 0, sizeof(g_sim_mappings));
    g_active_provider = &g_sim_provider;
}

void vnet_nat_shutdown(void) {
    runtime_memset(g_sim_mappings, 0, sizeof(g_sim_mappings));
    g_active_provider = &g_sim_provider;
}

BppNatStatus vnet_nat_discover(int timeout_ms) {
    if (!g_active_provider || !g_active_provider->discover) return NAT_STATUS_UNSUPPORTED;
    return g_active_provider->discover(g_active_provider->userdata, timeout_ms);
}

BppNatStatus vnet_nat_forward(int ext_port, int int_port, const char *proto, const char *desc, int lease_seconds) {
    if (!g_active_provider || !g_active_provider->add_mapping) return NAT_STATUS_UNSUPPORTED;
    return g_active_provider->add_mapping(g_active_provider->userdata, ext_port, int_port, proto, desc, lease_seconds);
}

BppNatStatus vnet_nat_unforward(int ext_port, const char *proto) {
    if (!g_active_provider || !g_active_provider->delete_mapping) return NAT_STATUS_UNSUPPORTED;
    return g_active_provider->delete_mapping(g_active_provider->userdata, ext_port, proto);
}

BppNatStatus vnet_nat_get_external_ip(char *out_ip, size_t max_len) {
    if (!g_active_provider || !g_active_provider->get_external_ip) return NAT_STATUS_UNSUPPORTED;
    return g_active_provider->get_external_ip(g_active_provider->userdata, out_ip, max_len);
}

BppNatStatus vnet_nat_get_status(int ext_port, const char *proto, char *out_info, size_t max_len) {
    if (!g_active_provider || !g_active_provider->query_status) return NAT_STATUS_UNSUPPORTED;
    return g_active_provider->query_status(g_active_provider->userdata, ext_port, proto, out_info, max_len);
}

void vnet_nat_set_provider(const VNetNatProvider *provider) {
    if (provider) {
        g_active_provider = provider;
    } else {
        g_active_provider = &g_sim_provider;
    }
}

void vnet_nat_reset_default_provider(void) {
    g_active_provider = &g_sim_provider;
}

void vnet_nat_enable_simulation(bool enable) {
    g_simulation_mode = enable;
    if (enable) {
        g_active_provider = &g_sim_provider;
    }
}
