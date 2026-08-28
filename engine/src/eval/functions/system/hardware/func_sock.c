// FILENAME: func_sock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memops.h, memops.c, sock_engine.h, sock_engine.c, string.h)
// NEEDS: libcore (strings.h, strings.c, strops.h, strops.c)
// NEEDS: libengine (func_sock.h, string.c)
// Implements SOCK built-in functions: SOCK.OPEN%, SOCK.ACCEPT%, SOCK.RECV$, SOCK.POLL%.
//
// ---- Includes ----

#include "eval/functions/system/hardware/func_sock.h"
#include "runtime/sock_engine.h"
#include "runtime/strings.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include <string.h>

BValue func_sock_open(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = -1;

    int type = SOCK_TYPE_TCP;
    if (argc >= 1 && argv[0].type == VAL_STRING) {
        const char *t_str = str_data(argv[0].as.string);
        if (runtime_strcasecmp(t_str, "UDP") == 0) type = SOCK_TYPE_UDP;
        else if (runtime_strcasecmp(t_str, "RAW") == 0) type = SOCK_TYPE_RAW;
    }
    int h = sock_open(type);
    res.as.number = (double)h;
    if (h < 0) {
        err->code = 52;
        err->message = "Failed to allocate socket handle";
    }
    return res;
}

BValue func_sock_accept(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = -1;

    if (argc < 1 || (argv[0].type != VAL_NUMBER && argv[0].type != VAL_INTEGER)) {
        err->code = 5;
        err->message = "Socket handle required for SOCK.ACCEPT%";
        return res;
    }
    int listen_h = (int)argv[0].as.number;
    char client_ip[64] = {0};
    int client_port = 0;
    int client_h = sock_accept(listen_h, client_ip, sizeof(client_ip), &client_port);
    res.as.number = (double)client_h;
    return res;
}

BValue func_sock_recv(VMContext *vm, int argc, BValue *argv, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (argc < 1 || (argv[0].type != VAL_NUMBER && argv[0].type != VAL_INTEGER)) {
        err->code = 5;
        err->message = "Socket handle required for SOCK.RECV$";
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    int handle = (int)argv[0].as.number;
    size_t max_len = 1024;
    if (argc >= 2 && (argv[1].type == VAL_NUMBER || argv[1].type == VAL_INTEGER)) {
        max_len = (size_t)argv[1].as.number;
    }
    int timeout_ms = 1000;
    if (argc >= 3 && (argv[2].type == VAL_NUMBER || argv[2].type == VAL_INTEGER)) {
        timeout_ms = (int)argv[2].as.number;
    }

    char buf[2048];
    size_t out_len = 0;
    BppError recv_err = sock_recv(handle, buf, (max_len < sizeof(buf)) ? max_len : sizeof(buf), &out_len, timeout_ms);
    if (recv_err.code != 0) {
        *err = recv_err;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }
    res.as.string = str_create(vm_get_str(vm), buf, out_len);
    return res;
}

BValue func_sock_poll(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0;

    if (argc < 1 || (argv[0].type != VAL_NUMBER && argv[0].type != VAL_INTEGER)) return res;
    int handle = (int)argv[0].as.number;
    int mask = SOCK_POLL_IN | SOCK_POLL_OUT;
    if (argc >= 2 && (argv[1].type == VAL_NUMBER || argv[1].type == VAL_INTEGER)) mask = (int)argv[1].as.number;
    int timeout_ms = 0;
    if (argc >= 3 && (argv[2].type == VAL_NUMBER || argv[2].type == VAL_INTEGER)) timeout_ms = (int)argv[2].as.number;

    int revents = sock_poll(handle, mask, timeout_ms);
    res.as.number = (double)revents;
    return res;
}

BValue func_sock_status(VMContext *vm, int argc, BValue *argv, BppError *err) {
    (void)vm;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 1;
    if (argc < 1 || (argv[0].type != VAL_NUMBER && argv[0].type != VAL_INTEGER)) return res;
    int handle = (int)argv[0].as.number;
    res.as.number = sock_has_pending_data(handle) ? 1.0 : 0.0;
    return res;
}
