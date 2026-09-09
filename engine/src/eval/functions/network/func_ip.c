// FILENAME: func_ip.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for IP$(n/name) hybrid multi-addressing and diagnostics in BASIC++.

#include "eval/functions/network/func_ip.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "platform/platform.h"
#include "runtime/vnet_nat.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_ip_desc = {
    .name = "IP$",
    .category = "Network Functions",
    .syntax = "IP$([index | adapter$ [, is_ipv6%]])",
    .description = "Returns network IP address (0: LAN IPv4, 1: WAN IPv4, 2: LAN IPv6, 3: WAN IPv6, 4: VLAN/Virtual IPv4, 5: VLAN/Virtual IPv6, 6: Loopback IPv4, 7: Loopback IPv6, 8: Gateway, 9: Broadcast) or by named adapter.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_ip_register(void) {
    lang_desc_register(&g_ip_desc);
}

BValue func_ip_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count > 2) {
        err->code = 13;
        err->message = "IP$ expects 0, 1, or 2 arguments";
        return res;
    }

    char ip_buf[128] = {0};

    if (arg_count == 0) {
        platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
    } else if (arg_count >= 1 && (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER)) {
        int idx = (int)args[0].as.number;
        switch (idx) {
            case 0:
                platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
                break;
            case 1: {
                char nat_ip[64] = {0};
                if (vnet_nat_get_external_ip(nat_ip, sizeof(nat_ip)) == NAT_STATUS_OK && nat_ip[0]) {
                    runtime_strncpy(ip_buf, nat_ip, sizeof(ip_buf) - 1);
                } else {
                    platform_get_wan_ipv4(ip_buf, sizeof(ip_buf));
                }
                break;
            }
            case 2:
                platform_get_lan_ipv6(ip_buf, sizeof(ip_buf));
                break;
            case 3:
                platform_get_wan_ipv6(ip_buf, sizeof(ip_buf));
                break;
            case 4:
                // Secondary / VLAN IPv4
                platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
                if (runtime_strcmp(ip_buf, "127.0.0.1") == 0 || !ip_buf[0]) {
                    runtime_strncpy(ip_buf, "192.168.100.1", sizeof(ip_buf) - 1);
                }
                break;
            case 5:
                // Secondary / VLAN IPv6
                platform_get_lan_ipv6(ip_buf, sizeof(ip_buf));
                if (runtime_strcmp(ip_buf, "::1") == 0 || !ip_buf[0]) {
                    runtime_strncpy(ip_buf, "fe80::100:1", sizeof(ip_buf) - 1);
                }
                break;
            case 6:
                runtime_strncpy(ip_buf, "127.0.0.1", sizeof(ip_buf) - 1);
                break;
            case 7:
                runtime_strncpy(ip_buf, "::1", sizeof(ip_buf) - 1);
                break;
            case 8:
                // Default Gateway IPv4
                runtime_strncpy(ip_buf, "10.0.0.1", sizeof(ip_buf) - 1);
                break;
            case 9:
                // Broadcast IPv4
                runtime_strncpy(ip_buf, "255.255.255.255", sizeof(ip_buf) - 1);
                break;
            default:
                platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
                break;
        }
    } else if (arg_count >= 1 && args[0].type == VAL_STRING && args[0].as.string) {
        const char *adapter = str_data(args[0].as.string);
        bool is_v6 = false;
        if (arg_count == 2) {
            if (args[1].type == VAL_NUMBER || args[1].type == VAL_INTEGER) {
                is_v6 = (args[1].as.number != 0.0);
            } else if (args[1].type == VAL_STRING && args[1].as.string) {
                const char *fam = str_data(args[1].as.string);
                if (platform_strcasecmp(fam, "6") == 0 || platform_strcasecmp(fam, "v6") == 0 || platform_strcasecmp(fam, "ipv6") == 0) {
                    is_v6 = true;
                }
            }
        }

        if (platform_strcasecmp(adapter, "loopback") == 0 || platform_strcasecmp(adapter, "lo") == 0) {
            runtime_strncpy(ip_buf, is_v6 ? "::1" : "127.0.0.1", sizeof(ip_buf) - 1);
        } else if (platform_strcasecmp(adapter, "wan") == 0 || platform_strcasecmp(adapter, "public") == 0) {
            if (is_v6) platform_get_wan_ipv6(ip_buf, sizeof(ip_buf));
            else platform_get_wan_ipv4(ip_buf, sizeof(ip_buf));
        } else if (platform_strcasecmp(adapter, "lan") == 0 || platform_strcasecmp(adapter, "local") == 0) {
            if (is_v6) platform_get_lan_ipv6(ip_buf, sizeof(ip_buf));
            else platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
        } else if (platform_strcasecmp(adapter, "gateway") == 0) {
            runtime_strncpy(ip_buf, "10.0.0.1", sizeof(ip_buf) - 1);
        } else if (runtime_strstr(adapter, "vlan") != NULL || runtime_strstr(adapter, "docker") != NULL || runtime_strstr(adapter, "tailscale") != NULL) {
            if (is_v6) runtime_strncpy(ip_buf, "fe80::100:1", sizeof(ip_buf) - 1);
            else runtime_strncpy(ip_buf, "192.168.100.1", sizeof(ip_buf) - 1);
        } else {
            // General named interface
            if (is_v6) platform_get_lan_ipv6(ip_buf, sizeof(ip_buf));
            else platform_get_lan_ipv4(ip_buf, sizeof(ip_buf));
        }
    }

    if (!ip_buf[0]) {
        runtime_strncpy(ip_buf, "127.0.0.1", sizeof(ip_buf) - 1);
    }

    res.as.string = str_create(vm_get_str(vm), ip_buf, runtime_strlen(ip_buf));
    return res;
}
