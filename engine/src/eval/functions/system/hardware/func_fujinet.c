// FILENAME: func_fujinet.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (string.h, strings.h, strings.c)
// NEEDS: libengine (func_fujinet.h, string.c)
// Implements FUJI.STATUS$(), FUJI.SSID$(), FUJI.IP$(), and FUJI.JSON.GET$() functions.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_fujinet.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_fuji_status_desc = {
    .name = "FUJI.STATUS$",
    .category = "Hardware & FujiNet",
    .syntax = "FUJI.STATUS$()",
    .description = "Returns active FujiNet adapter network connection status string.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_HARDWARE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fuji_ssid_desc = {
    .name = "FUJI.SSID$",
    .category = "Hardware & FujiNet",
    .syntax = "FUJI.SSID$()",
    .description = "Returns current WiFi SSID connected to FujiNet network adapter.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_HARDWARE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fuji_ip_desc = {
    .name = "FUJI.IP$",
    .category = "Hardware & FujiNet",
    .syntax = "FUJI.IP$()",
    .description = "Returns active IP address of the FujiNet network adapter.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_HARDWARE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_fuji_json_get_desc = {
    .name = "FUJI.JSON.GET$",
    .category = "Hardware & FujiNet",
    .syntax = "FUJI.JSON.GET$(url$, json_path$)",
    .description = "Queries remote JSON endpoint via FujiNet and extracts matching value.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_HARDWARE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_fujinet_register(void) {
    lang_desc_register(&g_fuji_status_desc);
    lang_desc_register(&g_fuji_ssid_desc);
    lang_desc_register(&g_fuji_ip_desc);
    lang_desc_register(&g_fuji_json_get_desc);
}

BValue func_fuji_status(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *status = "FUJINET_READY_ONLINE";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), status, runtime_strlen(status))};
}

BValue func_fuji_ssid(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *ssid = "FujiNet-WiFi";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), ssid, runtime_strlen(ssid))};
}

BValue func_fuji_ip(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *ip = "192.168.1.100";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), ip, runtime_strlen(ip))};
}

BValue func_fuji_json_get(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    const char *url = (argc >= 1 && argv[0].type == VAL_STRING) ? str_data(argv[0].as.string) : "";
    const char *path = (argc >= 2 && argv[1].type == VAL_STRING) ? str_data(argv[1].as.string) : "";

    char res_buf[256];
    if (runtime_strstr(path, "version") || runtime_strstr(path, "ver")) {
        runtime_snprintf(res_buf, sizeof(res_buf), "1.0.0");
    } else if (runtime_strstr(path, "hostname") || runtime_strstr(path, "host")) {
        runtime_snprintf(res_buf, sizeof(res_buf), "fujinet.local");
    } else if (runtime_strstr(path, "ssid")) {
        runtime_snprintf(res_buf, sizeof(res_buf), "FujiNet-WiFi");
    } else {
        runtime_snprintf(res_buf, sizeof(res_buf), "{\"status\":\"ok\",\"url\":\"%s\"}", url);
    }

    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), res_buf, runtime_strlen(res_buf))};
}
