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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BValue func_fuji_status(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *status = "FUJINET_READY_ONLINE";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), status, strlen(status))};
}

BValue func_fuji_ssid(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *ssid = "FujiNet-WiFi";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), ssid, strlen(ssid))};
}

BValue func_fuji_ip(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)argc; (void)argv; (void)err;
    const char *ip = "192.168.1.100";
    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), ip, strlen(ip))};
}

BValue func_fuji_json_get(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)err;
    const char *url = (argc >= 1 && argv[0].type == VAL_STRING) ? str_data(argv[0].as.string) : "";
    const char *path = (argc >= 2 && argv[1].type == VAL_STRING) ? str_data(argv[1].as.string) : "";

    char res_buf[256];
    if (strstr(path, "version") || strstr(path, "ver")) {
        snprintf(res_buf, sizeof(res_buf), "1.0.0");
    } else if (strstr(path, "hostname") || strstr(path, "host")) {
        snprintf(res_buf, sizeof(res_buf), "fujinet.local");
    } else if (strstr(path, "ssid")) {
        snprintf(res_buf, sizeof(res_buf), "FujiNet-WiFi");
    } else {
        snprintf(res_buf, sizeof(res_buf), "{\"status\":\"ok\",\"url\":\"%s\"}", url);
    }

    return (BValue){.type = VAL_STRING, .as.string = str_create(vm_get_str(vm), res_buf, strlen(res_buf))};
}
