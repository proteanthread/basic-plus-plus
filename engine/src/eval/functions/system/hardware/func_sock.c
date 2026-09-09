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
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_sock_open_desc = {
    .name = "SOCK.OPEN%", .category = "Hardware & Network", .syntax = "SOCK.OPEN%(proto$)",
    .description = "Opens a network socket of specified protocol ('TCP', 'UDP', 'RAW') and returns handle.",
    .error_summary = "Error 52 (Bad file number) on allocation failure", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_sock_accept_desc = {
    .name = "SOCK.ACCEPT%", .category = "Hardware & Network", .syntax = "SOCK.ACCEPT%(listen_h%)",
    .description = "Accepts an incoming connection on a listening socket handle and returns client socket handle.",
    .error_summary = "Error 5 (Illegal function call) on invalid handle", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_sock_recv_desc = {
    .name = "SOCK.RECV$", .category = "Hardware & Network", .syntax = "SOCK.RECV$(handle% [, max_len% [, timeout_ms%]])",
    .description = "Receives binary or text payload from a socket handle.",
    .error_summary = "Error 5 (Illegal function call) on invalid handle", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_sock_poll_desc = {
    .name = "SOCK.POLL%", .category = "Hardware & Network", .syntax = "SOCK.POLL%(handle% [, mask% [, timeout_ms%]])",
    .description = "Polls a socket handle for readability or writability events.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};
static const LangDesc g_sock_status_desc = {
    .name = "SOCK.STATUS%", .category = "Hardware & Network", .syntax = "SOCK.STATUS%(handle%)",
    .description = "Returns 1 if socket handle has pending data, 0 otherwise.",
    .error_summary = "None", .subsystem = SUBSYSTEM_HARDWARE, .safety = SAFETY_IO, .type = FEATURE_FUNCTION
};

void func_sock_register(void) {
    lang_desc_register(&g_sock_open_desc);
    lang_desc_register(&g_sock_accept_desc);
    lang_desc_register(&g_sock_recv_desc);
    lang_desc_register(&g_sock_poll_desc);
    lang_desc_register(&g_sock_status_desc);
}

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
