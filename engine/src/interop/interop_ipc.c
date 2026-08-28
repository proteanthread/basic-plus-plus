// FILENAME: interop_ipc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDS: libcore (interop_ipc.h, interop_jsonrpc.h, interop_jsonrpc.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_ipc within BASIC++.
//
// ---- Includes ----

#include "interop/interop_ipc.h"
#include "interop/interop_jsonrpc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct InteropIpcServer {
    InteropIpcMode mode;
    bool running;
};

InteropIpcServer* interop_ipc_create(InteropIpcMode mode, const char* address, int port) {
    InteropIpcServer* srv = (InteropIpcServer*)calloc(1, sizeof(InteropIpcServer));
    if (srv) {
        srv->mode = mode;
        srv->running = false;
    }
    return srv;
}

static void process_request(InteropIpcServer* server, const char* line) {
    JsonRpcRequest req = {0};
    JsonRpcResponse res = {0};

    if (!jsonrpc_parse_request(line, &req)) {
        jsonrpc_format_error(&res, -32700, "Parse error");
    } else {
        res.id = req.id;
        if (strncmp(req.method, "init", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "exec", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "eval", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "get_var", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "set_var", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "version", 128) == 0) {
            strncpy(res.result, "{\"version\":\"6.5\"}", sizeof(res.result) - 1);
        } else if (strncmp(req.method, "shutdown", 128) == 0) {
            strncpy(res.result, "{\"success\":true}", sizeof(res.result) - 1);
            server->running = false;
        } else {
            jsonrpc_format_error(&res, -32601, "Method not found");
        }
    }

    char out_buf[8192];
    jsonrpc_format_response(&res, out_buf, sizeof(out_buf));
    fprintf(stdout, "%s\n", out_buf);
    fflush(stdout);
}

void interop_ipc_run(InteropIpcServer* server) {
    if (!server) return;
    server->running = true;

    if (server->mode == IPC_MODE_STDIO) {
        char buffer[8192];
        while (server->running && fgets(buffer, sizeof(buffer), stdin)) {
            process_request(server, buffer);
        }
    }
}

void interop_ipc_stop(InteropIpcServer* server) {
    if (server) {
        server->running = false;
    }
}

void interop_ipc_destroy(InteropIpcServer* server) {
    if (server) {
        free(server);
    }
}

int interop_ipc_serve(VMContext *vm, int port) {
    if (!vm) {
        return 1;
    }
    InteropIpcMode mode = (port > 0) ? IPC_MODE_TCP : IPC_MODE_STDIO;
    InteropIpcServer *server = interop_ipc_create(mode, "127.0.0.1", port);
    if (!server) {
        return 1;
    }
    interop_ipc_run(server);
    interop_ipc_destroy(server);
    return 0;
}

