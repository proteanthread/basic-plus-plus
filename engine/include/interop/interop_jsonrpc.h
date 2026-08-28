// FILENAME: interop_jsonrpc.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (interop_ipc.c, interop_jsonrpc.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_jsonrpc within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_JSONRPC_H
#define INTEROP_JSONRPC_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int id;
    char method[128];
    char params[4096];
} JsonRpcRequest;

typedef struct {
    int id;
    char result[4096];
    int error_code;
    char error_message[256];
} JsonRpcResponse;

bool jsonrpc_parse_request(const char* json_str, JsonRpcRequest* req);
void jsonrpc_format_response(const JsonRpcResponse* res, char* buffer, size_t buf_size);
void jsonrpc_format_error(JsonRpcResponse* res, int error_code, const char* error_message);

#endif // INTEROP_JSONRPC_H
