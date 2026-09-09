// FILENAME: func_upnp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (builtins, sys_fn.c, string_fn.c)
// NEEDS: libcore (memory.h, strings.h), libserver (vnet_nat.h)
// Implements UPnP functions (UPNP.EXTERNALIP$, UPNP.STATUS, UPNP.STATUS$).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "eval/eval.h"
#include "runtime/vnet_nat.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/language_descriptor.h"
#include "types/types.h"
#include "vm/vm.h"

static const LangDesc g_upnp_externalip_desc = {
    .name = "UPNP.EXTERNALIP$",
    .category = "Network & UPnP",
    .syntax = "UPNP.EXTERNALIP$()",
    .description = "Returns active WAN external public IP address discovered via UPnP/IGD.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_upnp_status_desc = {
    .name = "UPNP.STATUS",
    .category = "Network & UPnP",
    .syntax = "UPNP.STATUS(port[, proto$])",
    .description = "Returns integer status of the specified UPnP port forwarding mapping.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_upnp_status_str_desc = {
    .name = "UPNP.STATUS$",
    .category = "Network & UPnP",
    .syntax = "UPNP.STATUS$(port[, proto$])",
    .description = "Returns descriptive status string of the specified UPnP port forwarding mapping.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_SERVER,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_upnp_register(void) {
    lang_desc_register(&g_upnp_externalip_desc);
    lang_desc_register(&g_upnp_status_desc);
    lang_desc_register(&g_upnp_status_str_desc);
}

BValue func_upnp_externalip_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    char ip_buf[64] = "127.0.0.1";
    vnet_nat_get_external_ip(ip_buf, sizeof(ip_buf));

    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), ip_buf, runtime_strlen(ip_buf));
    return res;
}

BValue func_upnp_status_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    int ext_port = 80;
    char proto[16] = "TCP";

    if (arg_count >= 1) {
        ext_port = (int)args[0].as.number;
    }
    if (arg_count >= 2) {
        if (args[1].type == VAL_STRING && args[1].as.string) {
            runtime_strncpy(proto, str_data(args[1].as.string), sizeof(proto) - 1);
            proto[sizeof(proto) - 1] = '\0';
            str_release(vm_get_str(vm), args[1].as.string);
        }
    }

    char status_buf[128] = "";
    BppNatStatus st = vnet_nat_get_status(ext_port, proto, status_buf, sizeof(status_buf));

    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    if (uname && uname[runtime_strlen(uname) - 1] == '$') {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), status_buf, runtime_strlen(status_buf));
    } else {
        res.type = VAL_NUMBER;
        res.as.number = (double)st;
    }
    return res;
}
